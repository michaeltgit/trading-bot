import trading_core as tc

def load_config():
    cfg = tc.Config()
    if not cfg.load("config.cfg"):
        raise RuntimeError("Failed to load config.cfg")

    symbols = [s.strip() for s in cfg.get_string("symbols").split(',')]
    max_position = cfg.get_int("risk.maxPosition")

    return cfg, symbols, max_position