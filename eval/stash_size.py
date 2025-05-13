import matplotlib.pyplot as plt

from fig_common import *

def avg_stash_size():
    pass

def max_stash_size(data): # maybe max stash size vs. threshold? 
    fig, ax = plt.subplots()

    # threshold = 32
    Z = 8 
    L = 4

    x_var = thresholds

    # threshold value
    ax.plot([min(x_var), max(x_var)], [min(x_var), max(x_var)], '--', color=ref_line_color)

    for i, d in enumerate(data):
        y_var = [d[L][Z][x].max_stash_size for x in x_var]    
        ax.plot(x_var, y_var, '.-', color=colors[i], label=list(labels.values())[i])

    ax.set_xlabel("Stash Eviction Trigger Threshold")
    ax.set_ylabel("Max. Stash Size")
    ax.set_title(f"Max. Stash Size vs. Eviction Threshold") # \nZ={Z}, threshold={threshold}")
    ax.legend()

    fig.savefig("max_stash_size.png")
    fig.savefig("max_stash_size.pdf")


if __name__=="__main__":
    data = []
    for label in labels:
        data.append(parse_logfile(f"./data/{label}.log")) # data[L][Z][thresh]

    max_stash_size(data)
