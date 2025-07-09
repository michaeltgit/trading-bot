from log import log_line, flush_log

def print_market_summary(symbol, spread, bid_total, ask_total, imbalance_percent):
    CYAN = '\033[96m'
    BOLD = '\033[1m'
    RESET = '\033[0m'

    lines = []
    lines.append(f"[{CYAN}{BOLD}{symbol}{RESET}] Most recent order book update impacted top 6 levels — recalculating summary")
    lines.append("Market Summary:")
    spread_percent = (spread / ((ask_total + bid_total) / 2)) * 100
    lines.append(f"- Spread: ${spread:.2f} ({spread_percent:.4f}%)")
    lines.append(f"- Bid Total (Top 6 levels): ${bid_total:.2f}")
    lines.append(f"- Ask Total (Top 6 levels): ${ask_total:.2f}")
    lines.append(f"- Order Book Imbalance: {imbalance_percent:.2f}%\n")

    for line in lines:
        log_line(symbol, line)

    flush_log(symbol)

def format_price_level(level):
    return f"{level.price:>10.2f} @ {level.size:<7.4f}"

def print_order_book(orderBook, tc, symbol, depth=6):
    lines = []
    width = 25
    total_width = width * 2 + 3

    title = " ORDER BOOK "
    dash_count = (total_width - len(title))
    title_line = "-" * dash_count + title + "-" * dash_count
    if len(title_line) < total_width:
        title_line += "-"
    lines.append(title_line)

    lines.append(f"{'BID PRICE @ SIZE':<{width}}| {'ASK PRICE @ SIZE':<{width}}")
    lines.append(f"{'-'*width}|{'-'*width}")

    bid_depth = orderBook.depth(tc.Side.Bid, depth)
    ask_depth = orderBook.depth(tc.Side.Ask, depth)

    for i in range(max(len(bid_depth), len(ask_depth))):
        bid = format_price_level(bid_depth[i]) if i < len(bid_depth) else ' ' * width
        ask = format_price_level(ask_depth[i]) if i < len(ask_depth) else ' ' * width
        lines.append(f"{bid:<{width}}| {ask:<{width}}")

    lines.append(f"{'-'*width}|{'-'*width}")

    for line in lines:
        log_line(symbol, line)

    flush_log(symbol)

    print("\n\n\n", end='')