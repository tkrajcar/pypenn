# Honor system.
# Copyright 2009 Tim Krajcar <allegro@conmolto.org

import db
import pennmush
import util
from ll import ansistyle

conn = None

def pts(victim):
    global conn
    if not conn:
        conn = db.get_conn()
    victim = int(victim.lstrip("#"))
    cursor = conn.cursor()
    cursor.execute("SELECT balance FROM honor_log WHERE victim = " + str(victim) + " ORDER BY dt desc LIMIT 1")
    if cursor.rowcount == 0: # no honor recorded as of yet - should rarely happen since new players will be assigned some
        return 0
    balance = cursor.fetchone()
    return balance[0]

def log_view(victim):
    (executor, caller, enactor) = pennmush.call_info()
    victim = int(victim.lstrip("#"))
    global conn
    if not conn:
        conn = db.get_conn()
    cursor = conn.cursor()
    cursor.execute("SELECT delta, balance, description, dt FROM honor_log WHERE victim = " + str(victim) + " ORDER BY dt desc LIMIT 20")
    pennmush.notify([enactor], util.titlebar("Honor Log: " + pennmush.api.name("#" + str(victim))))
    pennmush.notify([enactor], " Date/Time       Change  Balance  Description")
    for (delta, balance, description, dt) in cursor.fetchall():
        pennmush.notify([enactor], " " + dt.strftime("%d/%m/%y %H:%m") + "  " + str(delta).rjust(6) + "  " + str(balance).rjust(7) + "  " + str(description)[:44])
    pennmush.notify([enactor], util.footerbar())

def top_list(clan):
    (executor, caller, enactor) = pennmush.call_info()
    global conn
    if not conn:
        conn = db.get_conn()
    cursor = conn.cursor()
    pennmush.notify([enactor],"Not implemented at present.")
