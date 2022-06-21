from docplex.mp.model import Model

class MHS_cplex:

    def __init__(self, weights_dict:dict):
        self.m = Model()
        self.vars = {
            i : self.m.binary_var() for i in weights_dict
        } # vars[i] (= h_i) is a 0-1 variable telling if i is in the Hitting Set
        self.m.minimize(
            self.m.sum(weights_dict[i]*self.vars[i] for i in weights_dict)
        ) # minimize  c_1*h_1 + ... + c_n*h_n   (c_i = weights_dict[i])

    def hit(self, k):
        '''k is a set to be hit'''
        a = {}
        for i in self.vars:
            if (i in k): a[i] = 1
            else:        a[i] = 0
        self.m.add_constraint(
            self.m.sum(a[i]*self.vars[i] for i in self.vars) >= 1
        ) # a_j1*h_1 + ... + a_jn*h_n   (a_ji = a[i] = i∈k ; h_i = vars[i])

    def get(self):
        sol = self.m.solve()
        assert sol
        h = set()
        for i in self.vars:
            if sol.get_value(self.vars[i]) == 1:
                h.add(i)
        return h

if __name__ == '__main__':
    mhs = MHS_cplex({'a':1, 'b':2, 'c':3, 'd':4, 'e':4})
    #mhs.m.print_information()
    mhs.hit(['a', 'b'])
    mhs.hit(['d', 'e'])
    #mhs.hit(['e'])

    print(mhs.get())
'''
https://ibmdecisionoptimization.github.io/tutorials/html/Beyond_Linear_Programming.html#Integer-Optimization
If you're using a Community Edition of CPLEX runtimes, 
depending on the size of the problem, the solve stage may fail 
and will need a paying subscription or product installation.
'''
