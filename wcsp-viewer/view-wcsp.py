import argparse
import statistics as stat
from wcsp_reader import *
import numpy as np
import pandas as pd
from fpdf import FPDF
from pdfrw import PdfReader, PdfWriter, IndirectPdfDict

NO_FILE = 123

def getPathOut(arg, wcsp_input, num_instances, ext, suffix=''):
    if arg[-1] == '/' or num_instances > 1:
        dir = arg
        if arg[-1] != '/': dir += '/'
        os.system(f'mkdir -p {dir}')
        path = dir + os.path.basename(wcsp_input)
        path = os.path.splitext(path)[0] + suffix + ext
        return path
    else:
        return os.path.splitext(arg)[0] + ext
    
def statistics(list):
    mean = stat.mean(list)
    stdev = stat.pstdev(list, mu=mean)
    median = stat.median(list)
    return {'mean':mean, 'median':median, 'mean-2*stdev':mean-2*stdev, 'mean+2*stdev':mean+2*stdev}


global pdf_object
pdf_object = None

def init_pdf(title, wcsp_path):
    global pdf_object
    pdf_object = FPDF()
    pdf_object.add_page()
    pdf_object.set_font("Arial", size = 20)
    pdf_object.cell(200, 10, txt = title, ln = 1, align = 'C')
    pdf_object.set_font("Arial", size = 8)
    pdf_object.cell(200, 10, txt = wcsp_path, ln = 1, align = 'C')
    pdf_object.set_font("Arial", size = 10)
    pdf_object.cell(200, 10, txt = "", ln = 1, align = 'L')


def print_(*args): #print to terminal & to pdf
    print(*args)
    global pdf_object
    if pdf_object:
        txt = ' '.join(args).replace('\t', ' '*10)
        pdf_object.cell(200, 5, txt = txt, ln = 1, align = 'L')
    

parser = argparse.ArgumentParser()

parser.add_argument('wcsp_file', nargs='+', \
    help='input wcsp instance')

parser.add_argument('--preproc', '-p', metavar='FILE|DIR', \
    nargs='?', const='preproc', default=None, \
    help='Preprocess instance with toulbar2. \
            Preproessed instance(s) saved as FILE or in DIR/. \
            By default, saved as preproc.wcsp or in preproc/. \
            Remaining info will be computed from preprocessed instance(s).')
parser.add_argument('--report', '-r', metavar='PDF|DIR', \
    nargs='?', const='report', default=None, \
    help='Store info & plots to pdf (one file per instance) \
            Saved as PDF or in DIR/. \
            By default, saved as report.pdf or in report/.')

parser.add_argument('--csv', '-C', metavar='CSV', \
    nargs='?', const='excel.csv', default=None, \
    help='Store info to csv excel (one file with all instances). \
        Saved as CSV. By default, saved as excel.csv.')
#parser.add_argument('--csv_spaced', metavar='CSV', \
#    help='Store info to csv excel (values separated by comma & space)')

parser.add_argument('--domain', '-d', action='store_true', \
    help='Get domain sizes of each variable of the instance.')
parser.add_argument('--arity', '-a', action='store_true', \
    help='Get arity of each function of the instance.')
parser.add_argument('--hardness', '-H', action='store_true', \
    help='Get hardness of each function of the instance. \
        A function is either hard (costs 0 and top) or soft (costs not top) \
        or mixt (costs include top).')

parser.add_argument('--graph', '-g', metavar='PDF|DIR', nargs='?', const=NO_FILE, default=None, \
    help='Get interaction graph.  \
            If PDF or DIR given, figure(s) are saved there.')
parser.add_argument('--tree', '-t', metavar='PDF|DIR', nargs='?', const=NO_FILE, default=None, \
    help='Get tree decomposition of interaction graph.  \
            If PDF or DIR given, figure(s) are saved there.')
parser.add_argument('--scatter', '-s', metavar='PDF|DIR', nargs='?', const=NO_FILE, default=None, \
    help='Plot scatter graph showing (tuple) cost vs (function) arity. \
            If PDF or DIR given, figure(s) are saved there.')
parser.add_argument('--bar', '-b', metavar='PDF|DIR', nargs='?', const=NO_FILE, default=None, \
    help='Plot bar graph showing (function) hardness vs (function) arity. \
            If PDF or DIR given, figure(s) are saved there.')

parser.add_argument('--noshow', '-n', action='store_true', \
    help='Don\'t show plots. By default plots are shown (windows emerge) \
        if just one wcsp_file given.')

parser.add_argument('--undefaultness', '-u', action='store_true', \
    help='Get statistical indexs on the undefaultness of every function in the instance. \
        Let f be a function: \
        undefaultnss(f) = num_tuples_cost_not_default(f) / num_possible_tuples(f)')
parser.add_argument('--complexity', '-c', action='store_true', \
    help='Get statistical indexs on the complexity of every function in the instance. \
        Let f be a function: \
        complexity(f) = num_distinct_costs(f) / num_possible_tuples(f)')




args = parser.parse_args()
this_dir = os.path.dirname(os.path.abspath(__file__))
num_instances = len(args.wcsp_file)
first = True

for wcsp_input in args.wcsp_file:
    print()
    print()
    print(f"{wcsp_input} instance:")
    ident = "\t"
    if args.preproc:
        wcsp_preproc = getPathOut(args.preproc, wcsp_input, num_instances, '.wcsp')

        print(ident, 'Preprocessing...')
        os.system(f'{this_dir}/preproc.sh {wcsp_input} {wcsp_preproc}')
        print(ident*2, f'Preprocessed instance saved as {wcsp_preproc}')
        print(ident, 'Done.')
        print()
        wcsp = wcsp_preproc
    else:
        wcsp = wcsp_input

    print(ident, 'Computing information...')
    wr = WCSP_reader(wcsp)
    wr.traverse_instance()
    wr.compute_additional_info()
    print(ident, 'Done.')

    name = os.path.basename(wcsp)

    if args.report:
        init_pdf(name, wcsp)
        info_pdf = '.info.tmp.pdf'
        tmp_pdfs = [info_pdf]
        pdfs = [info_pdf]
    
    print()
    for label in wr.info: #print info
        print_(ident, f'{label}: {wr.info[label]}')

    info = {'name' : name}
    info.update(wr.info)

    if args.arity:
        print_()
        print_(ident, 'Arity:')
        for a in wr.arity_count:
            print_(ident*2, f'{wr.arity_count[a]} functions of arity {a}')
    if args.hardness:
        print_()
        print_(ident, 'Hardness:')
        for h in wr.hardness_count:
            print_(ident*2, f'{wr.hardness_count[h]} {h} functions')
    if args.domain:
        print_()
        print_(ident, 'Domain:')
        for s in wr.domain_count:
            print_(ident*2, f'{wr.domain_count[s]} domains of size {s}')
        
    if args.undefaultness:
        print_()
        print_(ident, 'Undefaultness:')
        print_(ident, '[let f be a function: undefaultness(f) = num_tuples_cost_not_default(f) / num_possible_tuples(f)]')
        d = statistics(wr.undefaultness)
        for label in d:
            print_(ident*2, f'{label}: {d[label]}')
            info[f'{label}(undefaultness)'] = d[label]
    if args.complexity:
        print_()
        print_(ident, 'Complexity:')
        print_(ident, '[let f be a function: complexity(f) = num_distinct_costs(f) / num_possible_tuples(f)]')
        d = statistics(wr.complexity)
        for label in d:
            print_(ident*2, f'{label}: {d[label]}')
            info[f'{label}(complexity)'] = d[label]

    if args.csv:
        if first:
            first = False
            df = pd.DataFrame(columns=[label for label in info])
        df = df.append(other=info, ignore_index=True)

    ### PLOT ###
    plot_list = [ \
        ('bar', args.bar, wr.plot_bar),
        ('scatter', args.scatter, wr.plot_scatter),
        ('graph', args.graph, wr.plot_graph),
        ('tree', args.tree, wr.plot_tree),        
    ]
    some_plot_done = False
    for label,arg,func in plot_list:
        if arg:
            if not some_plot_done:
                print()
                print(ident, 'Computing plots...')
                some_plot_done = True
            func()
            if arg != NO_FILE:
                pdf = getPathOut(arg, wcsp, num_instances, '.pdf', suffix=f'.{label}')
                plt.savefig(pdf)
                print(ident*2, f'{label} saved as {pdf}')
            elif args.report:
                pdf = f'.{label}.tmp.pdf'
                tmp_pdfs.append(pdf)
                plt.savefig(pdf)
            if args.report:
                pdfs.append(pdf)

    if some_plot_done:
        print(ident, 'Done.')

    if args.report:
        pdf_object.output(info_pdf)
        writer = PdfWriter()
        for inpfn in pdfs:
            writer.addpages(PdfReader(inpfn).pages)
        writer.write(getPathOut(args.report, wcsp, num_instances, '.pdf'))

        for pdf in tmp_pdfs:
            os.remove(pdf)
        
            
    
if num_instances == 1 and not args.noshow:
    plt.show()

if args.csv:
    df.to_csv(args.csv, index=False, header=True)
