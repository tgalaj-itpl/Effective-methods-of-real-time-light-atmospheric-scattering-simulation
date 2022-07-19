import numpy as np
import glm

def mie_phase_func(g: float, mu: float):
    return (3.0 * (1.0 - g*g) * (1.0 + mu*mu)) / (4.0 * np.pi * 2.0 * (2.0 + g*g) * glm.pow(1.0 + g*g - 2*g * mu, 1.5))

def rayleigh_phase_func(mu: float):
    return 3.0 * (1.0 + mu * mu) / (16.0 * np.pi)

def solve_quadratic(a: float, b: float, c: float):
    x1 = -1.0
    x2 = -1.0

    if b == 0:
        if a == 0:
            return False, x1, x2
        x1 = 0
        x2 = np.sqrt(-c / a)
        return True, x1, x2
    
    d = b * b - 4.0 * a * c

    if d < 0.0:
        return False, x1, x2

    q = 0.0
    if b < 0.0:
        q = -0.5 * (b - np.sqrt(d))
    else:
        q = -0.5 * (b + np.sqrt(d))

    x1 = q / a
    x2 = c / q

    return True, x1, x2

def ray_sphere_intersection(origin: glm.vec3, dir: glm.vec3, radius: float):
    t0 = 0.0
    t1 = 0.0
    
    A = glm.dot(dir, dir)
    B = 2.0 * glm.dot(dir, origin) 
    C = glm.dot(origin, origin) - (radius * radius)

    has_solution, t0, t1 = solve_quadratic(A, B, C)
    if not has_solution:
        return False, t0, t1

    if t0 > t1:
        t0, t1 = t1, t0 # swap values

    return True, t0, t1

# Print iterations progress
def printProgressBar (iteration, total, prefix = '', suffix = '', decimals = 1, length = 100, fill = '■', printEnd = "\r"):
    """
    Call in a loop to create terminal progress bar
    @params:
        iteration   - Required  : current iteration (Int)
        total       - Required  : total iterations (Int)
        prefix      - Optional  : prefix string (Str)
        suffix      - Optional  : suffix string (Str)
        decimals    - Optional  : positive number of decimals in percent complete (Int)
        length      - Optional  : character length of bar (Int)
        fill        - Optional  : bar fill character (Str)
        printEnd    - Optional  : end character (e.g. "\r", "\r\n") (Str)
    """
    percent = ("{0:." + str(decimals) + "f}").format(100 * (iteration / float(total)))
    filledLength = int(length * iteration // total)
    bar = fill * filledLength + '-' * (length - filledLength)
    print('\r%s |%s| %s%% %s' % (prefix, bar, percent, suffix), end = printEnd)
    # Print New Line on Complete
    if iteration == total: 
        print()
