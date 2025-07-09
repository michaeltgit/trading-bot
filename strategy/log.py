import threading
from collections import defaultdict
import datetime
import sys

_print_lock = threading.Lock()

_log_buffers = defaultdict(list)
_log_lock = threading.Lock()

_flush_lock = threading.Lock()

def log(message):
    with _print_lock:
        timestamp = datetime.datetime.now().strftime("[%Y-%m-%d %H:%M:%S.%f]")[:-3]
        print(f"{timestamp} {message}", file=sys.stdout, flush=True)

def buffer_log(symbol, message):
    with _log_lock:
        _log_buffers[symbol].append(message)

def flush_log(symbol):
    with _log_lock:
        lines = list(_log_buffers[symbol])
        _log_buffers[symbol].clear()

    if lines:
        with _flush_lock:
            for line in lines:
                log(line)

def start_logger():
    return None

def stop_logger():
    pass

def log_line(symbol, msg):
    full_msg = f"[{symbol}] | {msg}"
    buffer_log(symbol, full_msg)