from pysat.examples.rc2 import RC2
from pysat.formula import WCNF
from time import process_time, time
import sys

wcnf = WCNF(from_file=sys.argv[1])
p_start = process_time()
r_start = time()
rc2 = RC2(wcnf)
rc2.compute()
p_end = process_time()
r_end = time()
print(rc2.cost, p_end-p_start, r_end-r_start, sep=',')
