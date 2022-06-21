#from itertools import count
from pysat.solvers import Solver
from pysat.formula import WCNF
from mhs.MHS_adaptor import *

class MaxSAT:

    model = None
    cost = None
    violated_clauses_idx = None

    def cost_of(self, h):
        return sum([self.weights[x] for x in h])

    def __init__(self, file:str=None, wcnf:WCNF=None, algorithm='lb', hit_solver='hitman', \
        output='short', performance:dict={}):
        """
        file -- path to .wcnf file
        clauses -- WCNF object
            PRE: file==None XOR wcnf==None
        algorithm = 'lb' | 'lub'
        hit_solver = 'hitman' | 'cplex' | 'static' | 'dynamic' | 'dynamic+'
        output = 'none' | 'outcome' | 'short' | 'long' -- partial solutions printed to std out
        """
        self.algorithm = algorithm
        self.hit_solver = hit_solver
        self.out = output
        self.performance = performance

        assert((file==None) ^ (wcnf==None))
        if file!=None:
            F = WCNF(from_file=file)
        else:
            F = wcnf
        # F : WCNF -- F.soft, F.hard, F.wght

        self.block_idx = F.nv + 1 # index of the first blocking variable
        self.clauses = []        # self.clauses    = [c_i ∨ b_i          | c_i ∈ F.soft]
        self.block_lits = set()  # self.block_lits = {-b_i               | c_i ∈ F.soft}, blocking variables
        self.weights = {}        # self.weights    = {-b_i : weight(c_i) | c_i ∈ F.soft}
        for i in range(len(F.soft)): 
            self.clauses.append(F.soft[i] + [self.block_idx + i])
            self.block_lits.add(-(self.block_idx + i))
            self.weights[-(self.block_idx + i)] = F.wght[i]
        self.clauses.extend(F.hard)
        self.top_cost = F.topw
        self.hard_clauses = F.hard

        if self.out=='long':
            print('F.soft:', F.soft)
            print('F.hard:', F.hard)
            print('self.clauses:', self.clauses)
            print('self.block_lits:', self.block_lits)
            print('self.weights:', self.weights)

    
    def coreSAT(self, h:set):
        """
        PRE: h ⊆ block_lits 
        POST: if SAT(clauses U (block_lits - h)): return True
              else: return False, k
                where k is a core: k ⊆ (block_lits - h) and UNSAT(clauses U k)
        """
        self.performance['sat_calls'] += 1

        # assumptions = {-b_i | -b_i ∉ h}
        if self.sat_solver.solve(assumptions = self.block_lits.difference(h)):
            return True, None
        else:
            k = self.sat_solver.get_core()
            return False, k


    def solve(self):
        self.performance['mhs_calls'] = 0
        self.performance['sat_calls'] = 1
        solver_hard = Solver(bootstrap_with=self.hard_clauses)
        if not solver_hard.solve():
            if self.out != 'none':
                print('Hard clauses unsat')
            return False

        self.sat_solver = Solver(bootstrap_with=self.clauses)

        K = MHS_adaptor(weights=self.weights, solver=self.hit_solver, performance=self.performance)
        h = set()
        lb = 0
        if self.algorithm == 'lb':
            sat, k = self.coreSAT(h)
            while not sat and lb < self.top_cost:
                # K ⊆ Cores, h ∈ MHS(K)
                K.hit(k)
                h = K.getMHS()
                lb = self.cost_of(h)

                self.account_iteration(K, k, h, lb)
                sat, k = self.coreSAT(h)

        else: # self.algorithm == 'lub':
            ub = self.top_cost
            while lb < ub:
                # K ⊆ Cores, h ∈ HS(K)
                sat, k = self.coreSAT(h)
                if sat:
                    # minimum hitting set
                    ub = min(ub, self.cost_of(h))
                    h = K.getMHS()
                    lb = max(lb, self.cost_of(h))

                    self.account_iteration(K, k, h, lb, ub)
                else:
                    # hitting set
                    K.hit(k)
                    h = K.getHS()

                    self.account_iteration(K, k, h)

        if lb < self.top_cost:
            self.cost = lb
            self.model = self.sat_solver.get_model()[:-len(self.block_lits)]
            self.violated_clauses_idx = [-lit - self.block_idx for lit in h]
            if self.out != 'none': 
                print('Solution found with cost', self.cost)
            return True
        else:
            if self.out != 'none':
                print('The solution must have cost <', self.top_cost)
                print('Any solution has cost >=', lb)
            return False


        #let vc = [F.soft[i] for i in self.violated_clauses_idx]
        #    - vc ⊆ F.soft 
        #    - SAT(F - vc)
        #    - cost(vc) is minimum


    def account_iteration(self, K, k, h, lb=None, ub=None):
        self.performance['hitting_set_size'] = len(h)
        if self.out == 'long':
            sep = '\t'
            if lb != None: print('MHS', end='')
            else         : print(' HS', end='')
            print(f"{sep}k = {k}{sep}h = {h}", end="")
            if lb != None: print(f"{sep}lb = {lb}", end="")
            if ub != None: print(f"{sep}ub = {ub}", end="")
            print()
        elif self.out == 'short':
            sep = ',  '
            if lb != None: print('MHS', end='')
            else         : print(' HS', end='')
            print(f"{sep}{K.num_sets} cores found{sep}hitting set size = {len(h)}", end="")
            if lb != None: print(f"{sep}lower bound = {lb}", end="")
            if ub != None: print(f"{sep}upper bound = {ub}", end="")
            print()



if __name__ == '__main__':
    import argparse
    from time import process_time, time

    '''def list2str(list, max_len=10):
        if (len(list) <= max_len): 
            return str(list)
        else:
            s = '['
            for x in list[:max_len//2]: s += str(x) + ', '
            s += '... '
            for x in list[-max_len//2:]: s += ', ' + str(x)
            s += ']'
            return s'''

    parser = argparse.ArgumentParser()
    parser.add_argument("file", type=str, help="path to a .wcnf file")

    parser.add_argument("-m", "--model", action='store_true',
        help='print optimal model'
    )
    parser.add_argument("-a", "--algorithm", default='lub', 
        choices=['lb', 'lub'],
        help='default: lub'
    )
    parser.add_argument("-s", "--hit_solver", default='cplex', \
        choices=['hitman', 'cplex', 'static', 'dynamic'], #, 'dynamic+'],
        help='default: cplex'
    )

    parser.add_argument("-n", "--noprint", action='store_true',
        help='don\'t print intermediate solutions'
    )

    '''parser.add_argument("--pop", type=int, default=1, \
        choices=[1,2,3],
        help='only for hit solvers static, dynamic, dynamic+. Default: 1'
    )
    parser.add_argument("--sort", type=int, default=1, \
        choices=[1,2],
        help='only for hit solvers static, dynamic, dynamic+. Default: 1'
    )
    parser.add_argument('-v', '--verbose', action='count', default=0, \
        help='output verbosity (-v: outcome, -vv: short, -vvv: long)'
    )'''

    args = parser.parse_args()

    if args.noprint: out = 'none'
    else: out = 'short'

    p_start = process_time()
    r_start = time()
    cms = MaxSAT(file=args.file, \
        algorithm=args.algorithm, hit_solver=args.hit_solver, output=out)
    cms.solve()
    p_end = process_time()
    r_end = time()
    real_time = r_end - r_start
    cpu_time = p_end - p_start

    print()
    if args.model:
        print('optimal model:')
        for lit in cms.model:
            print(lit, end=' ')
        print()
        print()
    print(f'cost: {cms.cost}')
    print(f'real_time: {real_time}')
    print(f'cpu_time: {cpu_time}')

    '''print()
    rc2 = RC2(WCNF(from_file=args.file))
    rc2.compute()
    print(f'RC2 found model: {list2str(cms.model)}')
    print(f'      with cost: {rc2.cost}')'''

    #print(cms.violated_clauses_idx)
    #print([cms.clauses[i] for i in cms.violated_clauses_idx])


