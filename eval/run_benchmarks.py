
import os
import subprocess
import time

from fig_common import *

RUN_ALL=False

if __name__=="__main__":

    for label in labels.keys(): 
        output_file = f"./data/{label}.log"
        if RUN_ALL and os.path.exists(output_file):
            os.remove(output_file)

        for level in levels:
            for bucket_size in bucket_sizes:
                for threshold in thresholds:
                    client_cmd = ["../oram/benchmarks",
                                  str(label), str(level), str(bucket_size), str(threshold)]
                    server_cmd = ["../oram/server", str(level), str(bucket_size)]

                    if os.path.exists(output_file):
                        with open(output_file, 'r') as logfile:
                            contents = logfile.readlines()

                            if f"Testing L = {level}, Z = {bucket_size}, THRES = {threshold}\n" in contents:
                                continue

                    with open(output_file, 'a') as logfile:
                        client = subprocess.Popen(client_cmd, stdout=logfile)
                        time.sleep(1)
                        server = subprocess.Popen(server_cmd, stdout=subprocess.PIPE)

                        for line in server.stdout:
                            print(line)
                        
                    client.wait()
