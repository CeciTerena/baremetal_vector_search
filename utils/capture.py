import serial
import datetime
from pathlib import Path
from compare import BEGIN_RE, END_RE, PARAM_RE

LOG_FOLDER = Path(__file__).parent.parent / "logs"
BOOT_FOLDER = Path(__file__).parent.parent / "logs" / "boot"

PORT = "/dev/ttyUSB0"
BAUD_RATE = 115200 # REMEMBER TO KEEP IT UP TO DATE WITH THE REST OF THE CODEBASE!!!
TIMEOUT = 1.0

def save_log(output, name, boot=None):
    timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    log_file = LOG_FOLDER / f"{name}_{timestamp}.log"
    with open(log_file, "w") as f:
        f.writelines(output)
    print(f"DONE: {log_file.name} written to {LOG_FOLDER}")
    if boot:
        with open(BOOT_FOLDER / f"{name}_{timestamp}.boot.log", "w") as f:
            f.writelines(boot)

def main():
    LOG_FOLDER.mkdir(exist_ok=True)
    algo = None
    params = {}
    output = []
    boot = []

    with serial.Serial(PORT, baudrate=BAUD_RATE, exclusive=True, timeout=TIMEOUT) as ser:
        try:
            while True:
                data = ser.readline().decode(errors="replace")
                if not data:
                    continue
                line = data.rstrip("\r\n") + "\n"  # cause uart_print sends \r\n, but here just \n is enogh
                print(line, end="")
                output.append(line)
                if line.rstrip().endswith("START"):
                    boot = output[:-1]
                    output = [line]
                if m := BEGIN_RE.match(line):
                    algo = m.group("algo")
                    params = dict(PARAM_RE.findall(m.group("params")))
                elif END_RE.match(line):
                    if algo:
                        save_log(output, f"{algo}_{params['dataset']}_{params['platform']}", boot)
                    else:  # in case a capture start mid run.
                        print("END without BEGIN, capture NOT saved")
                    algo = None
                    params = {}
                    output = []
                    boot = []
        except KeyboardInterrupt:
            if algo:
                save_log(boot + output, f"partial_{algo}")

if __name__ == "__main__":
    main()
