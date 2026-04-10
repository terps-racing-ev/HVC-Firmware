import pandas as pd

def write_ocv_header(df, output_path):
    required_columns = {"Voltage [V]", "SOC [%]"}
    missing = required_columns - set(df.columns)
    if missing:
        raise ValueError(f"Missing required columns: {sorted(missing)}")
    
    df = df.sort_values("SOC [%]", ascending = False)
    entries = []
    for _, row in df[["Voltage [V]", "SOC [%]"]].dropna().iterrows():
        voltage_mv = int(round(float(row["Voltage [V]"]) * 1000))
        capacity_pct = int(round(float(row["SOC [%]"]) * 100))  # percent scaled by 100

        if voltage_mv < 0 or capacity_pct < 0:
            raise ValueError("Voltage and SOC must be non-negative")

        entries.append(f"    {{ {voltage_mv}u, {capacity_pct}u }},")

    header_text = "\n".join([
        "/** NOTE: THE OCV DATA DOES NOT ACCOUNT FOR VOLTAGE DROOP **/",
        "",
        "#include \"ocv_lookup_table.h\"",
        "",
        "static OCV_Voltage_Lookup_t OCV_Lookup_Table[] = {",
        *entries,
        "};",
        "",,
    ])

    with open(output_path, "w", encoding="utf-8", newline="\n") as f:
        f.write(header_text)
