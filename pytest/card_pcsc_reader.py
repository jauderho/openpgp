from smartcard import System
from smartcard.pcsc.PCSCExceptions import ListReadersException
from smartcard.pcsc.PCSCContext import PCSCContext

def _list_readers():
    try:
        return System.readers()
    except ListReadersException:
        # If the PCSC system has restarted the context might be stale, try
        # forcing a new context (This happens on Windows if the last reader is
        # removed):
        PCSCContext.instance = None
        return System.readers()

class CardReader(object):
    def __init__(self, dev):
        self.__dev = dev
        self.__conn = dev.createConnection()
        self.__conn.connect()

    def get_string(self, num):
        if num == 1:
            return self.__dev.name
        return ""

    def increment_seq(self):
        print("increment_seq")

    def reset_device(self):
        print("reset_device")

    def is_tpdu_reader(self):
        print("is_tpdu_reader")
        return False

    def ccid_get_result(self):
        print("ccid_get_result")

    def ccid_get_status(self):
        # always power on
        return 1

    def ccid_power_on(self):
        if self.__conn is None:
            self.__conn = self.__dev.createConnection()
            self.__conn.connect()
        return self.__conn.getATR()

    def ccid_power_off(self):
        pass

    def ccid_send_data_block(self, data):
        print("ccid_send_data_block")

    def ccid_send_cmd(self, data):
        print("ccid_send_cmd")

    def send_tpdu(self, info=None, more=0, response_time_ext=0,
                  edc_error=0, no_error=0):
        print("send_tpdu")

    def recv_tpdu(self):
        print("recv_tpdu")

    def send_cmd(self, cmd):
        return b""


def get_pcsc_device():
    readers = _list_readers()
    if len(readers) > 0:
        return CardReader(readers[0])
    else:
        return None
