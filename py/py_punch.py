from linalg import vec2
from pinworld import smoothstep
from math import sin, fabs

# context variables, updated on each frame run
c_size = vec2(1,1)
c_half_size = vec2(1,1)
c_time = 0.0


def meta_shade(pin_index, pin_pos, pin_value):
    
    speed = (c_size.x / 160) * 20.0
    time_pin = c_time * speed
    distance = fabs(time_pin - pin_index)
    distance = distance / 2.0
    distance = 1.0 - distance
        
    return distance