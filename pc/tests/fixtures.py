"""Sample LibreHardwareMonitor and ioreg outputs shared by the source tests."""

def sensor(name, value):
    return {"Text": name, "Value": value, "Children": []}


def group(name, *sensors):
    return {"Text": name, "Value": "", "Children": list(sensors)}


def hardware(name, *groups):
    return {"Text": name, "Value": "", "Children": list(groups)}


LHM_DESKTOP_WITH_AMD_GPU = {
    "Text": "Sensor",
    "Value": "Value",
    "Children": [
        {
            "Text": "DESKTOP-LOOM",
            "Value": "",
            "Children": [
                hardware(
                    "AMD Ryzen 7 5800X",
                    group("Temperatures", sensor("Core (Tctl/Tdie)", "61.5 °C"), sensor("CCD1 (Tdie)", "55,0 °C")),
                    group("Load", sensor("CPU Total", "23.4 %")),
                ),
                hardware(
                    "AMD Radeon RX 6700 XT",
                    group("Temperatures", sensor("GPU Core", "58.0 °C"), sensor("GPU Hot Spot", "70.0 °C")),
                    group("Load", sensor("GPU Core", "87.0 %"), sensor("D3D 3D", "85.0 %")),
                    group("SmallData", sensor("GPU Memory Used", "6144.0 MB"), sensor("GPU Memory Total", "12288.0 MB")),
                ),
                hardware(
                    "Intel(R) UHD Graphics 770",
                    group("Load", sensor("D3D 3D", "3.0 %")),
                    group("SmallData", sensor("D3D Dedicated Memory Used", "128.0 MB")),
                ),
            ],
        }
    ],
}

IOREG_APPLE_SILICON = """
+-o AGXAcceleratorG14X  <class AGXAcceleratorG14X, id 0x100000abc>
    {
      "PerformanceStatistics" = {"In use system memory (driver)"=0,"Alloc system memory"=1795162112,"Tiler Utilization %"=12,"Renderer Utilization %"=31,"Device Utilization %"=34,"In use system memory"=917504000}
    }
"""

IOREG_INTEL_MAC_WITH_RADEON = """
+-o AMDRadeonX6000_AMDNavi14GraphicsAccelerator
    {
      "PerformanceStatistics" = {"vramFreeBytes"=3221225472,"Device Utilization %"=57,"vramUsedBytes"=1073741824}
    }
+-o IntelAccelerator
    {
      "PerformanceStatistics" = {"Device Utilization %"=4}
    }
"""
