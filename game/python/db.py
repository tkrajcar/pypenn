# db.py
# Simple database interaction layer.
# Copyright 2009 Tim Krajcar <allegro@conmolto.org>

import MySQLdb
def get_conn():
    return MySQLdb.connect(host='localhost', user='e_mush', passwd='Wn5qnHyUhXmMTXbA', db='e_mush')

