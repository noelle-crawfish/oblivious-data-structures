
class DataPoint:
    def __init__(self, max_stash_size, avg_stash_size, bw_usage):
        self.max_stash_size = max_stash_size
        self.avg_stash_size = avg_stash_size
        self.bw_usage = bw_usage

def parse_config(config_print: str) -> dict:
    config = {}
    parts = config_print.split(', ')
    config['L'] = int(parts[0].split(' ')[3])
    config['Z'] = int(parts[1].split(' ')[2])
    config['thresh'] = int(parts[2].split(' ')[2])
    return config
    
def parse_logfile(filename: str) -> dict[dict[dict[DataPoint]]]:
    data = {}

    def init_dict_idx(i, j, k):
        if i not in data.keys():
            data[i] = {}
        if j not in data[i].keys():
            data[i][j] = {}

    num_data_points = 2
    with open(filename, 'r') as f:
        contents = f.readlines()
        contents = [contents[i:i+num_data_points+1] for i in range(0, len(contents), num_data_points+1)]
        for dp in contents:
            config = parse_config(dp[0])
            max_stash_size = int(dp[1].split(' ')[2])
            bw_usage = int(dp[2].split(' ')[2])

            init_dict_idx(config['L'], config['Z'], config['thresh'])
            data[config['L']][config['Z']][config['thresh']] = DataPoint(max_stash_size, 0, bw_usage)

        data = dict(sorted(data.items()))
        for L in data:
            data[L] = dict(sorted(data[L].items()))
            for Z in data[L]:
                data[L][Z] = dict(sorted(data[L][Z].items()))
    return data

colors = [
    '#ff0084', # rose
    '#631a86', # tekhelet (purple)
    '#3891a6', # blue (munseli)
    '#ffca3a', # sunglow (yellow)
    '#8ac926', # yellow green
]
ref_line_color = '#ab859b'

labels = {
    # "stack": "Stack",
    # "queue": "Queue",
    "avl": "AVL Tree (Map/Set)",
}

levels = [2, 3, 4, 5, 6]
# levels = [4]
# bucket_sizes = [2, 4, 8]
bucket_sizes = [8]
# thresholds = [4, 8, 16, 32, 64, 70, 80, 90, 100, 128]
thresholds = [70]

linestyles = ['.-', '.--', '.-.', '.:']
