#! /usr/bin/python3

import os
import sys
import io
import re
import time
import datetime
import array
import argparse
from py_pt3 import arib25
from py_pt3 import bcas
from py_pt3 import pt3


TABLE = [
    {"ch":  1, "freq":  0, "slot":0, "desc":""},
    {"ch":  2, "freq":  1, "slot":0, "desc":""},
    {"ch":  3, "freq":  2, "slot":0, "desc":""},
    {"ch":  4, "freq": 13, "slot":0, "desc":""},
    {"ch":  5, "freq": 14, "slot":0, "desc":""},
    {"ch":  6, "freq": 15, "slot":0, "desc":""},
    {"ch":  7, "freq": 16, "slot":0, "desc":""},
    {"ch":  8, "freq": 17, "slot":0, "desc":""},
    {"ch":  9, "freq": 18, "slot":0, "desc":""},
    {"ch": 10, "freq": 19, "slot":0, "desc":""},
    {"ch": 11, "freq": 20, "slot":0, "desc":""},
    {"ch": 12, "freq": 21, "slot":0, "desc":""},
    {"ch": 13, "freq": 63, "slot":0, "desc":""},
    {"ch": 14, "freq": 64, "slot":0, "desc":""},
    {"ch": 15, "freq": 65, "slot":0, "desc":""},
    {"ch": 16, "freq": 66, "slot":0, "desc":""},
    {"ch": 17, "freq": 67, "slot":0, "desc":""},
    {"ch": 18, "freq": 68, "slot":0, "desc":""},
    {"ch": 19, "freq": 69, "slot":0, "desc":""},
    {"ch": 20, "freq": 70, "slot":0, "desc":""},
    {"ch": 21, "freq": 71, "slot":0, "desc":""},
    {"ch": 22, "freq": 72, "slot":0, "desc":""},
    {"ch": 23, "freq": 73, "slot":0, "desc":""},
    {"ch": 24, "freq": 74, "slot":0, "desc":""},
    {"ch": 25, "freq": 75, "slot":0, "desc":""},
    {"ch": 26, "freq": 76, "slot":0, "desc":""},
    {"ch": 27, "freq": 77, "slot":0, "desc":""},
    {"ch": 28, "freq": 78, "slot":0, "desc":""},
    {"ch": 29, "freq": 79, "slot":0, "desc":""},
    {"ch": 30, "freq": 80, "slot":0, "desc":""},
    {"ch": 31, "freq": 81, "slot":0, "desc":""},
    {"ch": 32, "freq": 82, "slot":0, "desc":""},
    {"ch": 33, "freq": 83, "slot":0, "desc":""},
    {"ch": 34, "freq": 84, "slot":0, "desc":""},
    {"ch": 35, "freq": 85, "slot":0, "desc":""},
    {"ch": 36, "freq": 86, "slot":0, "desc":""},
    {"ch": 37, "freq": 87, "slot":0, "desc":""},
    {"ch": 38, "freq": 88, "slot":0, "desc":""},
    {"ch": 39, "freq": 89, "slot":0, "desc":""},
    {"ch": 40, "freq": 90, "slot":0, "desc":""},
    {"ch": 41, "freq": 91, "slot":0, "desc":""},
    {"ch": 42, "freq": 92, "slot":0, "desc":""},
    {"ch": 43, "freq": 93, "slot":0, "desc":""},
    {"ch": 44, "freq": 94, "slot":0, "desc":""},
    {"ch": 45, "freq": 95, "slot":0, "desc":""},
    {"ch": 46, "freq": 96, "slot":0, "desc":""},
    {"ch": 47, "freq": 97, "slot":0, "desc":""},
    {"ch": 48, "freq": 98, "slot":0, "desc":""},
    {"ch": 49, "freq": 99, "slot":0, "desc":""},
    {"ch": 50, "freq":100, "slot":0, "desc":""},
    {"ch": 51, "freq":101, "slot":0, "desc":""},
    {"ch": 52, "freq":102, "slot":0, "desc":""},
    {"ch": 53, "freq":103, "slot":0, "desc":""},
    {"ch": 54, "freq":104, "slot":0, "desc":""},
    {"ch": 55, "freq":105, "slot":0, "desc":""},
    {"ch": 56, "freq":106, "slot":0, "desc":""},
    {"ch": 57, "freq":107, "slot":0, "desc":""},
    {"ch": 58, "freq":108, "slot":0, "desc":""},
    {"ch": 59, "freq":109, "slot":0, "desc":""},
    {"ch": 60, "freq":110, "slot":0, "desc":""},
    {"ch": 61, "freq":111, "slot":0, "desc":""},
    {"ch": 62, "freq":112, "slot":0, "desc":""},
#BS
    {"ch":151, "freq":  0, "slot":0, "desc":"BS朝日"},
    {"ch":161, "freq":  0, "slot":1, "desc":"BS-TBS"},
    {"ch":191, "freq":  1, "slot":0, "desc":"WOWOW prime"},
    {"ch":171, "freq":  0, "slot":2, "desc":"BSジャパン"},
    {"ch":192, "freq":  2, "slot":0, "desc":"WOWOWライブ"},
    {"ch":193, "freq":  2, "slot":1, "desc":"WOWOWシネマ"},
    {"ch":201, "freq":  7, "slot":1, "desc":"スター・チャンネル2"},
    {"ch":202, "freq":  7, "slot":2, "desc":"スター・チャンネル3"},
    {"ch":236, "freq":  6, "slot":2, "desc":"BSアニマックス"},
    {"ch":256, "freq":  1, "slot":2, "desc":"ディズニー・チャンネル"},
    {"ch":211, "freq":  4, "slot":0, "desc":"BS11デジタル"},
    {"ch":200, "freq":  4, "slot":1, "desc":"スター・チャンネル1"},
    {"ch":222, "freq":  4, "slot":2, "desc":"TwellV"},
    {"ch":238, "freq":  5, "slot":0, "desc":"FOX bs238"},
    {"ch":241, "freq":  5, "slot":1, "desc":"BSスカパー！"},
    {"ch":231, "freq":  5, "slot":2, "desc":"放送大学テレビ1"},
    {"ch":232, "freq":  5, "slot":2, "desc":"放送大学テレビ2"},
    {"ch":233, "freq":  5, "slot":2, "desc":"放送大学テレビ3"},
    {"ch":531, "freq":  5, "slot":2, "desc":"放送大学ラジオ"},
    {"ch":141, "freq":  6, "slot":0, "desc":"BS日テレ"},
    {"ch":181, "freq":  6, "slot":1, "desc":"BSフジ"},
    {"ch":101, "freq":  7, "slot":0, "desc":"NHK-BS1"},
    {"ch":102, "freq":  7, "slot":0, "desc":"NHK-BS1臨時"},
    {"ch":103, "freq":  1, "slot":1, "desc":"NHK-BSプレミアム"},
    {"ch":910, "freq":  7, "slot":1, "desc":"ウェザーニュース(WNI)"},
    {"ch":291, "freq":  8, "slot":2, "desc":"NHK総合テレビジョン（東京）"},
    {"ch":292, "freq":  8, "slot":2, "desc":"NHK教育テレビジョン（東京）"},
    {"ch":294, "freq":  8, "slot":1, "desc":"日本テレビ"},
    {"ch":295, "freq":  8, "slot":1, "desc":"テレビ朝日"},
    {"ch":296, "freq":  8, "slot":1, "desc":"TBSテレビ"},
    {"ch":297, "freq":  8, "slot":1, "desc":"テレビ東京"},
    {"ch":298, "freq":  8, "slot":2, "desc":"フジテレビ"},
    {"ch":234, "freq":  9, "slot":0, "desc":"グリーンチャンネル"},
    {"ch":242, "freq":  9, "slot":1, "desc":"J SPORTS 1"},
    {"ch":243, "freq":  9, "slot":2, "desc":"J SPORTS 2"},
    {"ch":252, "freq": 10, "slot":0, "desc":"IMAGICA BS"},
    {"ch":244, "freq": 10, "slot":1, "desc":"J SPORTS 3"},
    {"ch":245, "freq": 10, "slot":2, "desc":"J SPORTS 4"},
    {"ch":251, "freq": 11, "slot":0, "desc":"BS釣りビジョン"},
    {"ch":255, "freq": 11, "slot":1, "desc":"日本映画専門チャンネル"},
    {"ch":258, "freq": 11, "slot":2, "desc":"D-Life"},
    {"ch":237, "freq": 12, "slot":0, "desc":"スター・チャンネル プラス"},
    {"ch":239, "freq": 12, "slot":0, "desc":"日本映画専門チャンネルHD"},
    {"ch":306, "freq": 12, "slot":0, "desc":"フジテレビNEXT"},
    {"ch":100, "freq": 13, "slot":0, "desc":"e2プロモ"},
    {"ch":256, "freq": 13, "slot":0, "desc":"J sports ESPN"},
    {"ch":312, "freq": 13, "slot":0, "desc":"FOX"},
    {"ch":322, "freq": 13, "slot":0, "desc":"スペースシャワーTV"},
    {"ch":331, "freq": 13, "slot":0, "desc":"カートゥーンネットワーク"},
    {"ch":294, "freq": 13, "slot":0, "desc":"ホームドラマチャンネル"},
    {"ch":334, "freq": 13, "slot":0, "desc":"トゥーン・ディズニー"},
    {"ch":221, "freq": 14, "slot":0, "desc":"東映チャンネル"},
    {"ch":222, "freq": 14, "slot":0, "desc":"衛星劇場"},
    {"ch":223, "freq": 14, "slot":0, "desc":"チャンネルNECO"},
    {"ch":224, "freq": 14, "slot":0, "desc":"洋画★シネフィル・イマジカ"},
    {"ch":292, "freq": 14, "slot":0, "desc":"時代劇専門チャンネル"},
    {"ch":238, "freq": 14, "slot":0, "desc":"スター・チャンネル クラシック"},
    {"ch":310, "freq": 14, "slot":0, "desc":"スーパー！ドラマTV"},
    {"ch":311, "freq": 14, "slot":0, "desc":"AXN"},
    {"ch":250, "freq": 14, "slot":0, "desc":"スカイ・Asports＋"},
    {"ch": 55, "freq": 15, "slot":0, "desc":"ショップ チャンネル"},
    {"ch":335, "freq": 15, "slot":0, "desc":"キッズステーションHD"},
    {"ch":228, "freq": 16, "slot":0, "desc":"ザ・シネマ"},
    {"ch":800, "freq": 16, "slot":0, "desc":"スカチャンHD800"},
    {"ch":801, "freq": 16, "slot":0, "desc":"スカチャン801"},
    {"ch":802, "freq": 16, "slot":0, "desc":"スカチャン802"},
    {"ch":260, "freq": 17, "slot":0, "desc":"ザ・ゴルフ・チャンネル"},
    {"ch":303, "freq": 17, "slot":0, "desc":"テレ朝チャンネル"},
    {"ch":323, "freq": 17, "slot":0, "desc":"MTV 324ch：大人の音楽専門TV◆ミュージック・エア"},
    {"ch":352, "freq": 17, "slot":0, "desc":"朝日ニュースター"},
    {"ch":353, "freq": 17, "slot":0, "desc":"BBCワールドニュース"},
    {"ch":354, "freq": 17, "slot":0, "desc":"CNNj"},
    {"ch":110, "freq": 17, "slot":0, "desc":"ワンテンポータル"},
    {"ch":251, "freq": 18, "slot":0, "desc":"J sports 1"},
    {"ch":252, "freq": 18, "slot":0, "desc":"J sports 2"},
    {"ch":253, "freq": 18, "slot":0, "desc":"J sports Plus"},
    {"ch":254, "freq": 18, "slot":0, "desc":"GAORA"},
    {"ch":305, "freq": 19, "slot":0, "desc":"チャンネル銀河"},
    {"ch":333, "freq": 19, "slot":0, "desc":"アニメシアターX(AT-X)"},
    {"ch":342, "freq": 19, "slot":0, "desc":"ヒストリーチャンネル"},
    {"ch":290, "freq": 19, "slot":0, "desc":"TAKARAZUKA SKYSTAGE"},
    {"ch":803, "freq": 19, "slot":0, "desc":"スカチャン803"},
    {"ch":804, "freq": 19, "slot":0, "desc":"スカチャン804"},
    {"ch":240, "freq": 20, "slot":0, "desc":"ムービープラスHD"},
    {"ch":262, "freq": 20, "slot":0, "desc":"ゴルフネットワーク"},
    {"ch":314, "freq": 20, "slot":0, "desc":"LaLa HDHV"},
    {"ch":258, "freq": 21, "slot":0, "desc":"フジテレビONE"},
    {"ch":302, "freq": 21, "slot":0, "desc":"フジテレビTWO"},
    {"ch":332, "freq": 21, "slot":0, "desc":"アニマックス"},
    {"ch":340, "freq": 21, "slot":0, "desc":"ディスカバリーチャンネル"},
    {"ch":341, "freq": 21, "slot":0, "desc":"アニマルプラネット"},
    {"ch":160, "freq": 22, "slot":0, "desc":"C-TBSウェルカムチャンネル"},
    {"ch":161, "freq": 22, "slot":0, "desc":"QVC"},
    {"ch":185, "freq": 22, "slot":0, "desc":"プライム365.TV"},
    {"ch":293, "freq": 22, "slot":0, "desc":"ファミリー劇場"},
    {"ch":301, "freq": 22, "slot":0, "desc":"TBSチャンネル"},
    {"ch":304, "freq": 22, "slot":0, "desc":"ディズニー・チャンネル"},
    {"ch":325, "freq": 22, "slot":0, "desc":"MUSIC ON! TV"},
    {"ch":351, "freq": 22, "slot":0, "desc":"TBSニュースバード"},
    {"ch":343, "freq": 22, "slot":0, "desc":"ナショナルジオグラフィックチャンネル"},
    {"ch":257, "freq": 23, "slot":0, "desc":"日テレG+ HD"},
    {"ch":291, "freq": 23, "slot":0, "desc":"fashiontv"},
    {"ch":300, "freq": 23, "slot":0, "desc":"日テレプラス"},
    {"ch":315, "freq": 23, "slot":0, "desc":"FOXプラス"},
    {"ch":321, "freq": 23, "slot":0, "desc":"MusicJapan TV"},
    {"ch":350, "freq": 23, "slot":0, "desc":"日テレNEWS24"},
    {"ch":362, "freq": 23, "slot":0, "desc":"旅チャンネル"},
]


#crc32テーブル作成
def make_crc32_table():
    ret = array.array('L', ([0]*256))
    for j in range(256):
        crc = j << 24
        for i in range(8):
            if crc & 0x80000000:
                crc = (crc << 1) ^ 0x04c11db7
            else:
                crc = crc << 1
            crc &= 0xFFFFFFFF
        ret[j] = crc
    return ret

#crc計算
def crc32(datas, table=make_crc32_table()):
    crc = 0xFFFFFFFF
    for data in datas:
        crc = (crc << 8) ^ table[((crc >> 24 ) ^ data) & 0xFF]
        crc = crc & 0xFFFFFFFF
    return crc


def get_descriptors(data, data_length):
    descriptors = []
    i = 0
    while i < data_length:
        desc_tag = data[i]
        desc_len = data[i+1]
        desc     = data[i+2: i+2+desc_len]
        i += desc_len+2
        if desc_tag == 9:
            pid = get_pid(desc[2], desc[3])
        descriptors.append((desc_tag, desc_len, desc))
    return descriptors


def get_pid(hight, low):
    return ((hight & 0x1F) <<8) | low


def get_length12(hight, low):
    return ((hight & 0x0F) <<8) | low


class TsHeader:
    def __init__(self, packet):
        self.sync_byte                    = packet[0]
        self.transport_error_indicator    = (packet[1] & 0x80) >> 7
        self.payload_unit_start_indicator = (packet[1] & 0x40) >> 6
        self.transport_priority           = (packet[1] & 0x20) >> 5
        self.pid                          = get_pid(packet[1], packet[2])
        self.transport_scrambling_control = (packet[3] & 0xC0) >> 6
        self.adaptation_field_control     = (packet[3] & 0x20) >> 5
        self.payload_field_control        = (packet[3] & 0x10) >> 4
        self.continuity_counter           = (packet[3] & 0x0F)
        self.pointer_field                = -1
        if self.payload_unit_start_indicator == 1:
            self.pointer_field = packet[4]

    def get_section_part(self, packet):
        if self.payload_unit_start_indicator == 1:
            return packet[4+1+self.pointer_field:]
        else:
            return packet[4:]

    def to_bytes(self):
        data = bytearray(4)
        data[0] = self.sync_byte
        data[1] = ((self.transport_error_indicator    << 7) |
                   (self.payload_unit_start_indicator << 6) |
                   (self.transport_priority           << 5) |
                   (self.pid >> 8) & 0x1F                   )
        data[2] = self.pid & 0xFF
        data[3] = ((self.transport_scrambling_control << 6) |
                   (self.adaptation_field_control     << 5) |
                   (self.payload_field_control        << 4) |
                    self.continuity_counter                 )
        return bytes(data)



class TsSectionBase:
    default_packet_size = 188
    to_section_length_member = 3
    section_base_size   = 8
    crc_byte_size       = 4
    def __init__(self, section):
        self.table_id                 = section[0]
        self.section_syntax_indicator = ((section[1] & 0x80) >> 7)
        self.section_length           = get_length12(section[1], section[2])
        self.service_id               = (section[3] << 8) | section[4]
        self.version_number           = (section[5] & 0x3E) >> 1
        self.current_next_indicator   = section[5] & 0x01
        self.section_number           = section[6]
        self.last_section_number      = section[7]
        self.rest_size                = 0
        self.current_section_size     = 0
        self.section_data             = bytes(section)
        self._calc_rest_size()

    def _calc_rest_size(self):
        self.current_section_size = len(self.section_data)
        rest_size = (self.to_section_length_member+self.section_length) - self.current_section_size
        if rest_size < 0: self.rest_size = 0
        else: self.rest_size = rest_size

    def join_partial_data(self, rest_data):
        self.section_data = self.section_data + rest_data
        self._calc_rest_size()
        return self

    def get_rest_size(self):
        return self.rest_size

    def to_bytes(self):
        data = bytearray(self.section_base_size)
        data[0] = self.table_id
        data[1] = ((self.section_syntax_indicator << 7) |
                    0x30                                |
                    (self.section_length & 0xF0)  >> 8  )
        data[2] = self.section_length & 0xFF
        data[3] = (self.service_id >> 8) & 0xFF
        data[4] = self.service_id & 0xFF
        data[5] = ( 0xC0 |
                   ((self.version_number & 0x1F) << 1) |
                   (self.current_next_indicator & 0x01))
        data[6] = self.section_number
        data[7] = self.last_section_number
        return bytes(data)

    def get_depend_data_size(self):
        return ( self.to_section_length_member
               + self.section_length
               - self.crc_byte_size )

    def make_section(self, klass):
        return klass(self.section_data)

class TsPatSection(TsSectionBase):
    from_section_length_member = 5
    network_service_id = 0
    def __init__(self, section):
        super().__init__(section)
        self.program_numbers  = []
        self.program_map_pids = []
        self.network_pid      = 0
        pids_length = (self.section_length -
                        (self.from_section_length_member + self.crc_byte_size))

        pgsection = section[self.section_base_size:]
        i = 0
        while i < pids_length:
            program_number = (pgsection[i] << 8) | pgsection[i+1]
            progran_id     = get_pid(pgsection[i+2], pgsection[i+3])
            #if program_number == 0xFFFF: break
            self.program_numbers.append(program_number)
            self.program_map_pids.append(progran_id)
            if program_number == 0:
                self.network_pid = progran_id
            i += 4

    def update_program_numbers(self, program_numbers=None, program_map_pids=None):
        if program_numbers and program_map_pids:
            self.program_numbers  = program_numbers
            self.program_map_pids = program_map_pids
        else:
            select_id, select_pid  = next(((n, p) for n, p in zip(self.program_numbers, self.program_map_pids) if n != 0), None)
            self.program_numbers  = [0, select_id]
            self.program_map_pids = [self.network_pid, select_pid]

        self.section_length   = ( self.from_section_length_member
                                + self.crc_byte_size
                                + len(self.program_numbers) * 4)

    def get_service_pid(self, service_id):
        try:
            index = self.program_numbers.index(service_id)
            return (service_id, self.program_map_pids[index])
        except:
            return next(((n,p) for n,p in zip(self.program_numbers, self.program_map_pids) if n != 0), -1)


    def to_bytes(self):
        section_size = ( self.section_base_size
                       + len(self.program_numbers) * 4
                       + self.crc_byte_size )
        data = bytearray(section_size)

        base_data = super().to_bytes()
        data[:len(base_data)] = base_data[:]
        pgsection_start = self.section_base_size
        i = 0
        for n, p in zip(self.program_numbers, self.program_map_pids ):
            data[pgsection_start+i+0] = (n >> 8) & 0xFF
            data[pgsection_start+i+1] = n & 0xFF
            data[pgsection_start+i+2] = ((p >> 8) & 0x1F) | 0xE0
            data[pgsection_start+i+3] = p & 0xFF
            i += 4

        data_size = self.get_depend_data_size()
        crc = crc32(data[:data_size])
        data[data_size+0] = (crc >> 24) & 0xFF
        data[data_size+1] = (crc >> 16) & 0xFF
        data[data_size+2] = (crc >> 8 ) & 0xFF
        data[data_size+3] = crc & 0xFF
        return bytes(data)



class TsPmtSection(TsSectionBase):
    def __init__(self, section):
        super().__init__(section)
        self.stream_ids          = []
        self.pcr_pid             = get_pid(section[8], section[9])
        self.program_info_length = get_length12(section[10], section[11])
        start = 12
        self.program_descriptors = get_descriptors(section[start:], self.program_info_length)
        stream_length = (self.section_length -
                        (9 + self.program_info_length + self.crc_byte_size))

        stream_start = start + self.program_info_length
        stream_section = section[stream_start: stream_start+stream_length]
        i = 0
        while i < stream_length:
            stream_type    = stream_section[i]
            elementary_pid = get_pid(stream_section[i+1], stream_section[i+2])
            es_info_length = get_length12(stream_section[i+3], stream_section[i+4])
            es_descriptors = get_descriptors(stream_section[i+5: i+5+es_info_length], es_info_length)
            self.stream_ids.append((stream_type, elementary_pid, es_info_length, es_descriptors))
            i += es_info_length + 5

    def necessary_pids_for_record(self):
        #ECM
        ecm = [get_pid(desc[2], desc[3]) for tag, len, desc in self.program_descriptors if tag == 9 and len >= 4]
        tmp = [pid for stream_type, pid, _, _ in self.stream_ids if stream_type != 0x0D]
        tmp.extend(ecm)
        tmp.append(self.pcr_pid)
        return set(tmp)



class TsFilter:
    def __init__(self, service_id):
        self.service_id = service_id
        self.target_pmt_pid = -1
        self.target_pids = None
        self.pat_target_pids = None
        self.partial_pmt = None
        self.want_pmt = False

    def filter(self, packets):
        output = io.BytesIO()
        write = output.write
        count = len(packets)//188
        for i in range(count):
            data = packets[i*188:(i+1)*188]
            header = TsHeader(data)
            section = header.get_section_part(data)
            #PAT
            if header.pid == 0:
                pat = TsPatSection(section)
                (channel, self.target_pmt_pid) = pat.get_service_pid(self.service_id)
                pat.update_program_numbers([0, channel], [pat.network_pid, self.target_pmt_pid])
                self.pat_target_pids = {0}.union({n for n in pat.program_map_pids})
                tmpdata = pat.to_bytes()
                data = bytes(data[:5]) + tmpdata[:] + bytes([0XFF]*(188-5-len(tmpdata)))
                self.want_pmt = True
                write(data)
                continue
            #PMT
            elif self.want_pmt and header.pid == self.target_pmt_pid:
                if header.payload_unit_start_indicator == 1:
                    base = TsSectionBase(section)
                    if base.get_rest_size() > 0:
                        self.partial_pmt = dict(header=header, base=base)
                        write(data)
                        continue
                else:
                    if not self.partial_pmt:
                        write(data)
                        continue
                    if self.partial_pmt["header"].continuity_counter == header.continuity_counter:
                        write(data)
                        continue
                    else:
                        base = self.partial_pmt["base"].join_partial_data(section)
                        if base.get_rest_size() > 0:
                            write(data)
                            continue

                pmt = base.make_section(TsPmtSection)
                pids = pmt.necessary_pids_for_record()
                self.target_pids = self.pat_target_pids.union(pids)
                self.partial_pmt = None
                self.want_pmt = False

            if self.target_pids != None and header.pid in self.target_pids:
                write(data)

        output.flush()
        ret = bytes(output.getvalue())
        output.close()
        return ret


def main(args):
    #地上波
    #pt3video2
    #pt3video3
    #px4video2
    #px4video3
    #衛星波
    #pt3video0
    #pt3video1
    #px4video0
    #px4video1

    channel_info = next((c for c in TABLE if c['ch'] == args.channel), None)
    if not channel_info:
        print("チャンネルがありません\n")
        return

    with pt3.open(args.device, 188*816) as dev:

        comb = TsFilter(args.channel)
        #b25オブジェクト作成
        b25 = arib25.Arib25()

        b25.set_multi2_round(4)
        b25.set_strip(1)
        b25.set_emm_proc(0)

        cas = bcas.Bcas()
        cas.init()
        b25.set_b_cas_card(cas)

        if args.output:
            output = open(args.output, mode="wb")
            write = output.write
        else:
            output = None
            sys.stdout = os.fdopen(sys.stdout.fileno(), 'wb', buffering=0)
            write = sys.stdout.write

        dev.start()
        dev.set_channel(channel_info['freq'], channel_info['slot'])


        delta = datetime.timedelta(seconds=10)
        if args.period:
            m = re.match(r"(?:(?P<H>\d+)H)?(?:(?P<M>\d+)M)?(?:(?P<S>\d+)S)?", args.period)
            if m:
                seconds = int(m.group("S")) if m.group("S") else 0
                minutes = int(m.group("M")) if m.group("M") else 0
                hours   = int(m.group("H")) if m.group("H") else 0
                delta = datetime.timedelta(seconds=seconds, minutes=minutes, hours=hours)
        start_date = datetime.datetime.now()
        while True:
            if datetime.datetime.now() - start_date > delta:
                break
            packet = dev.read()
            if len(packet) <= 0: continue

            b25.put(packet)
            packet = b25.get()
            if len(packet) <= 0:
                continue

            data = comb.filter(packet)
            write(data)

        dev.stop()
        dev.close()

        if output:
            output.close()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
                  description="arib25ライブラリのPython3ラッパーのテストスクリプト")
    parser.add_argument("-c", "--channel",
                    help= "チャンネル(地上波放送)またはサービスID(衛星放送)", type=int)
    parser.add_argument("-s", "--satelite", help="衛星放送視聴", action="store_true")
    parser.add_argument("-d", "--device", help="デバイス名")
    parser.add_argument("-o", "--output", help="ファイル名設定されていなければ標準出力")
    parser.add_argument("-p", "--period", help="視聴時間 *H*M*s")
    parser.add_argument("-v", "--voltage", help="lnb電圧(0, 11, 15)",
                            choices=[0,11,15], default=0, type=int)
    parser.add_argument("-l", "--list", help="チャンネルリスト", action="store_true")

    args = parser.parse_args()

    if not args.channel:
        if args.list:
            channel_list = ["ch:{:03d} {}".format(c["ch"], c["desc"]) for c in TABLE if c["desc"]]
            print("\n".join(channel_list))
        else:
            parser.print_help()
    elif args.device:
        main(args)

