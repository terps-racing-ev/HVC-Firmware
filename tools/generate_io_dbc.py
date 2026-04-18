#!/usr/bin/env python3
"""Generate a DBC file for HVC-originated CAN messages.

The script reads CAN IDs from Core/Inc/Config/can_id.h and emits a DBC with
messages packed exactly as in firmware.
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path


ROOT_DIR = Path(__file__).resolve().parents[1]
DEFAULT_CAN_ID_HEADER = ROOT_DIR / "Core" / "Inc" / "Config" / "can_id.h"
DEFAULT_STATE_HEADER = ROOT_DIR / "Core" / "Inc" / "Data" / "state.h"
DEFAULT_OUTPUT = ROOT_DIR / "io_messages.dbc"

CAN_ID_NAMES = (
    "CAN_ID_IO_SUMMARY",
    "CAN_ID_STATE",
    "CAN_ID_ERRORED_PANIC",
    "CAN_ID_IO_CURRENT",
    "CAN_ID_SOC",
    "CAN_ID_ACC_SUMMARY",
    "CAN_ID_IO_VSENSE",
    "CAN_ID_CURRENT_LIMIT",
)


def _parse_can_ids(header_path: Path) -> dict[str, int]:
    """Extract selected CAN ID #defines from the header file."""
    text = header_path.read_text(encoding="utf-8")
    pattern = re.compile(
        r"^\s*#define\s+(CAN_ID_IO_SUMMARY|CAN_ID_STATE|CAN_ID_ERRORED_PANIC|CAN_ID_IO_CURRENT|CAN_ID_SOC|CAN_ID_ACC_SUMMARY|CAN_ID_IO_VSENSE|CAN_ID_CURRENT_LIMIT)\s+"
        r"(0x[0-9A-Fa-f]+|\d+)\b",
        re.MULTILINE,
    )

    can_ids: dict[str, int] = {}
    for name, raw_value in pattern.findall(text):
        can_ids[name] = int(raw_value, 0)

    missing = [name for name in CAN_ID_NAMES if name not in can_ids]
    if missing:
        missing_csv = ", ".join(missing)
        raise ValueError(
            f"Missing CAN ID define(s) in {header_path}: {missing_csv}"
        )

    return can_ids


def _parse_error_bits(state_header_path: Path) -> list[tuple[str, int]]:
    """Extract ErrorBit enum entries and resolved bit positions."""
    text = state_header_path.read_text(encoding="utf-8")
    enum_match = re.search(
        r"typedef\s+enum\s*\{(?P<body>.*?)\}\s*ErrorBit\s*;",
        text,
        re.DOTALL,
    )
    if enum_match is None:
        raise ValueError(f"Could not find ErrorBit enum in {state_header_path}")

    body = enum_match.group("body")
    body = re.sub(r"/\*.*?\*/", "", body, flags=re.DOTALL)

    token_pattern = re.compile(
        r"^\s*([A-Za-z_][A-Za-z0-9_]*)\s*(?:=\s*(.+?))?\s*,?\s*$"
    )

    def _parse_enum_value(value_expr: str) -> int:
        expr = value_expr.strip()

        literal = re.fullmatch(r"(0x[0-9A-Fa-f]+|\d+)[uUlL]*", expr)
        if literal is not None:
            return int(literal.group(1), 0)

        shift = re.fullmatch(
            r"(0x[0-9A-Fa-f]+|\d+)[uUlL]*\s*<<\s*(0x[0-9A-Fa-f]+|\d+)[uUlL]*",
            expr,
        )
        if shift is not None:
            lhs = int(shift.group(1), 0)
            rhs = int(shift.group(2), 0)
            return lhs << rhs

        raise ValueError(
            f"Unsupported ErrorBit enum value expression in {state_header_path}: {value_expr}"
        )

    errors: list[tuple[str, int]] = []
    next_value = 0
    for raw_line in body.splitlines():
        line = raw_line.split("//", 1)[0].strip()
        if not line:
            continue

        match = token_pattern.match(line)
        if match is None:
            continue

        name = match.group(1)
        explicit_value = match.group(2)
        if explicit_value is not None:
            next_value = _parse_enum_value(explicit_value)

        if name in {"BMS_ERR_COUNT", "BMS_ERR_DEFAULT"}:
            break
        if name.startswith("BMS_ERR_"):
            errors.append((name, next_value))

        next_value += 1

    if not errors:
        raise ValueError(f"No BMS_ERR_* entries found in {state_header_path}")

    return errors


def _dbc_frame_id(can_id: int) -> int:
    """Return DBC frame ID for an extended (29-bit) CAN identifier."""
    if can_id < 0 or can_id > 0x1FFFFFFF:
        raise ValueError(f"CAN ID out of 29-bit range: 0x{can_id:X}")
    return can_id | 0x80000000


def _error_signal_name(error_name: str) -> str:
    """Convert enum name (BMS_ERR_*) to a compact DBC signal name."""
    base = error_name
    if base.startswith("BMS_ERR_"):
        base = base[len("BMS_ERR_"):]

    parts = [part for part in base.split("_") if part]
    return "Err_" + "".join(part.title() for part in parts)


def _generate_dbc_text(
    io_summary_id: int,
    state_id: int,
    errored_panic_id: int,
    io_current_id: int,
    soc_id: int,
    acc_summary_id: int,
    io_vsense_id: int,
    current_limit_id: int,
    error_bits: list[tuple[str, int]],
    include_raw_error_mask: bool,
    node_name: str,
) -> str:
    """Build DBC content for HVC-originated messages."""
    io_summary_dbc_id = _dbc_frame_id(io_summary_id)
    state_dbc_id = _dbc_frame_id(state_id)
    errored_panic_dbc_id = _dbc_frame_id(errored_panic_id)
    io_current_dbc_id = _dbc_frame_id(io_current_id)
    soc_dbc_id = _dbc_frame_id(soc_id)
    acc_summary_dbc_id = _dbc_frame_id(acc_summary_id)
    io_vsense_dbc_id = _dbc_frame_id(io_vsense_id)
    current_limit_dbc_id = _dbc_frame_id(current_limit_id)

    error_signal_lines: list[str] = []
    error_value_lines: list[str] = []
    for error_name, bit in error_bits:
        if bit < 0 or bit > 31:
            raise ValueError(f"Error bit out of range for ErrorMask: {error_name}={bit}")

        signal_name = _error_signal_name(error_name)
        start_bit = 8 + bit
        error_signal_lines.append(
            f' SG_ {signal_name} : {start_bit}|1@1+ (1,0) [0|1] "" Vector__XXX'
        )
        error_value_lines.append(
            f'VAL_ {state_dbc_id} {signal_name} 0 "Inactive" 1 "Active" ;'
        )

    error_signals_block = "\n".join(error_signal_lines)
    error_values_block = "\n".join(error_value_lines)
    raw_error_mask_line = ""
    if include_raw_error_mask:
        raw_error_mask_line = ' SG_ BMS_ErrorMask : 8|32@1+ (1,0) [0|4294967295] "" Vector__XXX\n'

    return f'''VERSION ""

NS_ :
    NS_DESC_
    CM_
    BA_DEF_
    BA_
    VAL_
    CAT_DEF_
    CAT_
    FILTER
    BA_DEF_DEF_
    EV_DATA_
    ENVVAR_DATA_
    SGTYPE_
    SGTYPE_VAL_
    BA_DEF_SGTYPE_
    BA_SGTYPE_
    SIG_TYPE_REF_
    VAL_TABLE_
    SIG_GROUP_
    SIG_VALTYPE_
    SIGTYPE_VALTYPE_
    BO_TX_BU_
    BA_DEF_REL_
    BA_REL_
    BA_DEF_DEF_REL_
    BU_SG_REL_
    BU_EV_REL_
    BU_BO_REL_
    SG_MUL_VAL_

BS_:

BU_: {node_name}

BO_ {io_summary_dbc_id} IO_Summary: 7 {node_name}
 SG_ SDC_Closed : 0|1@1+ (1,0) [0|1] "" Vector__XXX
 SG_ IMD_Ok : 1|1@1+ (1,0) [0|1] "" Vector__XXX
 SG_ BMS_Fault_Ok : 2|1@1+ (1,0) [0|1] "" Vector__XXX
 SG_ Ref_Temp_C : 40|16@1- (0.01,0) [-327.68|327.67] "degC" Vector__XXX

BO_ {io_current_dbc_id} IO_Current: 8 {node_name}
 SG_ Current_Low_mA : 0|32@1- (1,0) [-2147483648|2147483647] "mA" Vector__XXX
 SG_ Current_High_mA : 32|32@1- (1,0) [-2147483648|2147483647] "mA" Vector__XXX

BO_ {io_vsense_dbc_id} IO_VSense: 8 {node_name}
 SG_ Batt_Voltage_mV : 0|32@1+ (1,0) [0|4294967295] "mV" Vector__XXX
 SG_ Inv_Voltage_mV : 32|32@1+ (1,0) [0|4294967295] "mV" Vector__XXX

BO_ {state_dbc_id} BMS_State: 5 {node_name}
 SG_ BMS_State : 0|8@1+ (1,0) [0|255] "" Vector__XXX
{raw_error_mask_line}{error_signals_block}

BO_ {errored_panic_dbc_id} Errored_Panic: 0 {node_name}

BO_ {soc_dbc_id} SOC: 8 {node_name}
 SG_ SOC_Percent : 0|16@1+ (0.01,0) [0|100] "%" Vector__XXX
 SG_ SOC_Capacity_As : 16|16@1+ (1,0) [0|65535] "A*s" Vector__XXX
 SG_ SOC_Delta_As : 32|32@1- (1,0) [-2147483648|2147483647] "A*s" Vector__XXX

BO_ {acc_summary_dbc_id} ACC_Summary: 8 {node_name}
 SG_ Acc_Volt_Min_mV : 0|16@1+ (1,0) [0|65535] "mV" Vector__XXX
 SG_ Acc_Volt_Max_mV : 16|16@1+ (1,0) [0|65535] "mV" Vector__XXX
 SG_ Acc_Temp_Min_C : 32|16@1- (0.1,0) [-3276.8|3276.7] "degC" Vector__XXX
 SG_ Acc_Temp_Max_C : 48|16@1- (0.1,0) [-3276.8|3276.7] "degC" Vector__XXX

BO_ {current_limit_dbc_id} Current_Limit: 8 {node_name}
 SG_ Negative_Current_Limit_mA : 0|32@1+ (1,0) [0|4294967295] "mA" Vector__XXX
 SG_ Positive_Current_Limit_mA : 32|32@1+ (1,0) [0|4294967295] "mA" Vector__XXX

VAL_ {state_dbc_id} BMS_State 0 "PRE_INIT" 1 "RUNNING" 2 "CHARGING" 3 "BALANCING" 4 "ERRORED" ;
{error_values_block}
'''


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Generate a DBC file for IO manager CAN messages."
    )
    parser.add_argument(
        "--can-id-header",
        type=Path,
        default=DEFAULT_CAN_ID_HEADER,
        help=f"Path to can_id.h (default: {DEFAULT_CAN_ID_HEADER})",
    )
    parser.add_argument(
        "--state-header",
        type=Path,
        default=DEFAULT_STATE_HEADER,
        help=f"Path to state.h (default: {DEFAULT_STATE_HEADER})",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=DEFAULT_OUTPUT,
        help=f"Output DBC file path (default: {DEFAULT_OUTPUT})",
    )
    parser.add_argument(
        "--node-name",
        default="HVC",
        help="Transmitter node name in the DBC (default: HVC)",
    )
    parser.add_argument(
        "--include-raw-error-mask",
        action="store_true",
        help=(
            "Include 32-bit BMS_ErrorMask signal in the state message. "
            "This overlaps with per-bit decoded error signals and may break strict DBC parsers."
        ),
    )
    args = parser.parse_args()

    can_id_header = args.can_id_header.resolve()
    state_header = args.state_header.resolve()
    output_path = args.output.resolve()

    can_ids = _parse_can_ids(can_id_header)
    error_bits = _parse_error_bits(state_header)
    dbc_text = _generate_dbc_text(
        io_summary_id=can_ids["CAN_ID_IO_SUMMARY"],
        state_id=can_ids["CAN_ID_STATE"],
        errored_panic_id=can_ids["CAN_ID_ERRORED_PANIC"],
        io_current_id=can_ids["CAN_ID_IO_CURRENT"],
        soc_id=can_ids["CAN_ID_SOC"],
        acc_summary_id=can_ids["CAN_ID_ACC_SUMMARY"],
        io_vsense_id=can_ids["CAN_ID_IO_VSENSE"],
        current_limit_id=can_ids["CAN_ID_CURRENT_LIMIT"],
        error_bits=error_bits,
        include_raw_error_mask=args.include_raw_error_mask,
        node_name=args.node_name,
    )

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(dbc_text, encoding="utf-8", newline="\n")

    print(f"Generated DBC: {output_path}")
    print(
        f"Used CAN IDs: CAN_ID_IO_SUMMARY=0x{can_ids['CAN_ID_IO_SUMMARY']:08X}, "
        f"CAN_ID_STATE=0x{can_ids['CAN_ID_STATE']:08X}, "
        f"CAN_ID_ERRORED_PANIC=0x{can_ids['CAN_ID_ERRORED_PANIC']:08X}, "
        f"CAN_ID_IO_CURRENT=0x{can_ids['CAN_ID_IO_CURRENT']:08X}, "
        f"CAN_ID_SOC=0x{can_ids['CAN_ID_SOC']:08X}, "
        f"CAN_ID_ACC_SUMMARY=0x{can_ids['CAN_ID_ACC_SUMMARY']:08X}, "
        f"CAN_ID_IO_VSENSE=0x{can_ids['CAN_ID_IO_VSENSE']:08X}, "
        f"CAN_ID_CURRENT_LIMIT=0x{can_ids['CAN_ID_CURRENT_LIMIT']:08X}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
