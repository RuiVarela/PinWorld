from linalg import vec2
from pinworld import smoothstep
from math import sin, fabs

# context variables, updated on each frame run
c_size = vec2(1,1)
c_half_size = vec2(1,1)
c_time = 0.0


def meta_shade(pin_index, pin_pos, pin_value):
    
    speed = 5.0
    wave_size = 4.0
    
    additional_size = (wave_size * 2.0) / c_size.x
    v_size = c_size.x * (1.0 + additional_size)
    
    time_pin = c_time * speed
    time_pin = time_pin / v_size
    time_pin = time_pin - int(time_pin)
    time_pin = time_pin * v_size - wave_size 
    
    
    distance = fabs(time_pin - pin_pos.x)
    distance = distance / wave_size
    distance = 1.0 - distance
        
    return distance