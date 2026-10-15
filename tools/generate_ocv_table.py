import pandas as pd

def write_ocv_header(df, output_path):
    required_columns = {"OCV [V]", "SOC [%]"}
    missing = required_columns - set(df.columns)
    if missing:
        raise ValueError(f"Missing required columns: {sorted(missing)}")
    
    df = df.sort_values("SOC [%]", ascending = False)
    entries = []
    for _, row in df[["OCV [V]", "SOC [%]"]].dropna().iterrows():
        voltage_mv = int(round(float(row["OCV [V]"]) * 1000))
        capacity_pct = int(round(float(row["SOC [%]"]) * 100))  # percent scaled by 100

        if voltage_mv < 0 or capacity_pct < 0:
            raise ValueError("OCV and SOC must be non-negative")

        entries.append(f"    {{ {voltage_mv}u, {capacity_pct}u }},")

    header_text = "\n".join([
        "/** NOTE: THE OCV DATA DOES NOT ACCOUNT FOR VOLTAGE DROOP **/",
        "",
        "#include \"ocv_lookup_table.h\"",
        "",
        "OCV_Voltage_Lookup_t OCV_Lookup_Table[] = {",
        *entries,
        "};",
        "",
        "const uint32_t OCV_Lookup_Table_Size = sizeof(OCV_Lookup_Table)/sizeof(OCV_Voltage_Lookup_t);",
        "",
    ])

    with open(output_path, "w", encoding="utf-8", newline="\n") as f:
        f.write(header_text)
