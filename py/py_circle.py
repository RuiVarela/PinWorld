from linalg import vec2
from pinworld import smoothstep
from math import sin, fabs

# context variables, updated on each frame run
c_size = vec2(1,1)
c_half_size = vec2(1,1)
c_time = 0.0
 

def debug_state():
    print(f"c_time={c_time} c_size={c_size} c_half_size={c_half_size}")

# debug_state()
# debug_state()

def meta_shade(pin_index, pin_pos, pin_value):
    out = 0.0
    size = 0.1
    radius = 0.7
    
    pp = pin_pos + vec2(0.0, sin(c_time * 0.25) * c_size.x * 0.25)


    normalized = (pp - c_half_size) / c_half_size.y
    distance = normalized.length()
    out = smoothstep(size, 0.0, fabs(radius - distance))

    return out