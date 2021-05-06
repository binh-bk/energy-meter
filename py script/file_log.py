# Binh Nguyen, Dec 2020
# log data to file


import os
import csv
import time
import json

def host_folder(sensor, json_=False):
    """designate a folder to save each month"""

    this_month_folder = time.strftime('%b%Y')
    basedir = os.path.abspath(os.path.dirname(__file__))
    all_dirs = [d for d in os.listdir(basedir) if os.path.isdir(d)]
    if len(all_dirs) == 0 or this_month_folder not in all_dirs:
        os.makedirs(this_month_folder)
        print('created: {}'.format(this_month_folder))
    if json:
        return os.path.join(basedir, this_month_folder, f'{sensor}.txt')
    return os.path.join(basedir, this_month_folder, f'{sensor}.csv')

def to_log(data, json_=True):
    '''write a json stream to a CSV format'''

    sensor = data['sensor']
    fname = host_folder(sensor, json_)
    file_existed = os.path.isfile(fname)
    headers = list(data.keys())

    if json:
        fname = host_folder(data['sensor'], json_=json_)
        with open(host_folder(sensor), 'a') as f:
            f.write(json.dumps(data))
            f.write('\n')
    else:
        with open(fname, 'a') as csvfile:
            writer = csv.DictWriter(
                csvfile, delimiter=',', lineterminator='\n',fieldnames=headers)
            if not file_existed:
                writer.writeheader()
            writer.writerow(data)

    return None


if __name__ == '__main__':
    print('Testing area')
