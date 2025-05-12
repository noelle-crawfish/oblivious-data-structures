import matplotlib.pyplot as plt

from fig_common import *

def avg_stash_size():
    pass

def max_stash_size(data): # maybe max stash size vs. threshold? 
    fig, ax = plt.subplots()

    threshold = 8
    Z = 8

    x_var = data[0].keys()

    # threshold value
    ax.plot([min(x_var), max(x_var)], [70, 70], '--', color=ref_line_color)

    for i, d in enumerate(data):
        y_var = [d[x][Z][threshold].max_stash_size for x in x_var]    
        ax.plot(x_var, y_var, '.-', color=colors[i], label=list(labels.values())[i])

    ax.set_xlabel("Path ORAM Height (L)")
    ax.set_ylabel("Max. Stash Size")
    ax.set_title(f"Max. Stash Size vs. Path ORAM Height\nZ={Z}, threshold={threshold}")
    ax.legend()

    fig.savefig("max_stash_size.png")


if __name__=="__main__":
    data = []
    for label in labels:
        data.append(parse_logfile(f"./data/{label}.log")) # data[L][Z][thresh]

    max_stash_size(data)
