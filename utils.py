def color(t):
    r = t/1000*256
    g = 256-r
    b = 0
    return r, g, b

print(color(218))