import os

def isDigit(c):
    return '0' <= c and c <= '9'

def compute_info(wcsp_file:str):
    tmp1 = '.1.info.tmp'
    tmp2 = '.2.info.tmp'
    t = 1
    found = False
    while not found and t<150:
        os.system(f'toulbar2 -B=1 -O=-3 -z=2 -A -timer={t} {wcsp_file} > {tmp1}')
        with open(tmp1) as f:
            found = (f.read().find('Connected components:') >= 0)
        t *= 2
    if not found:
        print('WARNING: unable to find additional information')
        return {}

    with open(tmp1) as f:
        out = f.read()
        info = {}
        
        i = out.find('non-unary cost functions')
        if i >= 0:
            while not isDigit(out[i]): i-=1
            end = i+1
            while isDigit(out[i]): i-=1
            begin = i+1
            info['Number of non-unary cost functions'] = out[begin:end]
            #print(out[begin:end])
        else: print('WARNING: nun-unary not found')
            
        i = out.find('Connected components:')
        if i >= 0:
            #print(out)
            i += len('Connected components:')
            while out[i] != ')': i+=1
            i+=1
            begin = i
            while isDigit(out[i]): i+=1
            end = i
            info['Number of connected components'] = out[begin:end]
            #print('beg_end '+out[begin:end])
        else: print('WARNING: connected components not found')


        #out = open(path_wcsp+'.out2').read()
        
        
    for label in ['Initial lower and upper bounds','Tree decomposition width','Tree decomposition height','Number of clusters']:
        os.system(f'cat {tmp1} | grep "{label}" > {tmp2}')
        with open(tmp2) as f:
            line = f.read()
            i = line.find(':')
            info[label] = line[i+2:-1]

    #Initial lower and upper bounds: [7959513, 103511264[ 92.310%
    #Tree decomposition width  : 54
    #Tree decomposition height : 55
    #Number of clusters        : 1

    os.system('rm -rf '+os.path.basename(wcsp_file)+'.info')
    os.system('rm -f problem.wcsp')
    os.system('rm -f problem.wcsp.dot')
    os.system('rm -f problem.wcsp.degree')
    os.system('rm -f '+tmp1)
    os.system('rm -f '+tmp2)

    return info

if __name__ == '__main__':
    import sys
    info = compute_info(sys.argv[1])
    print(info)


