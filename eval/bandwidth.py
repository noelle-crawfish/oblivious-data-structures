
import matplotlib.pyplot as plt

from fig_common import *

BLOCK_SIZE = 10240

def bw_vs_bucket_size(data):
    fig, ax = plt.subplots()

    threshold = 70
    # Z = 8 
    L = 4

    x_var = bucket_sizes

    setup_cost = 0
    for i in range(L):
        setup_cost += (1 << i)

    # threshold value
    ax.plot(x_var, [(2*L*x)+setup_cost*x for x in x_var], '--', color=ref_line_color)

    for i, d in enumerate(data):
        y_var = [((d[L][x][threshold].bw_usage + setup_cost) / d[L][x][threshold].rw_ops) for x in x_var]    
        ax.plot(x_var, y_var, '.-', color=colors[i], label=list(labels.values())[i])

    ax.set_xlabel("Bucket Capacity (ORAM Blocks)")
    ax.set_ylabel("Ratio of Total Bytes to Useful Bytes R/W")
    ax.set_title(f"BW Ratio vs. Path ORAM Bucket Capacity") # \nZ={Z}, threshold={threshold}")
    ax.legend()

    fig.savefig("bw_vs_bucket_size.png")
    fig.savefig("bw_vs_bucket_size.pdf")

def bw_vs_height(data):
    fig, ax = plt.subplots()

    threshold = 70
    Z = 8 

    x_var = levels

    setup_cost = [sum([1 << i for i in range(L)]) for L in x_var]
    # threshold value
    ax.plot(x_var, [(2*x*Z) + sc*Z for sc, x in zip(x_var, setup_cost)], '--', color=ref_line_color)

    for i, d in enumerate(data):
        y_var = [(d[x][Z][threshold].bw_usage+sc) / d[x][Z][threshold].rw_ops for sc, x in enumerate(x_var)]    
        ax.plot(x_var, y_var, '.-', color=colors[i], label=list(labels.values())[i])

    ax.set_xlabel("Height of Path ORAM Tree")
    ax.set_ylabel("Ratio of Total Bytes to Useful Bytes R/W")
    ax.set_title(f"BW Ratio vs. Path ORAM Tree Height") # \nZ={Z}, threshold={threshold}")
    ax.legend()

    fig.savefig("bw_vs_height.png")
    fig.savefig("bw_vs_height.pdf")

if __name__=="__main__":
    data = []
    for label in labels:
        data.append(parse_logfile(f"./data/{label}.log")) # data[L][Z][thresh]

    bw_vs_bucket_size(data)
    bw_vs_height(data)
    
