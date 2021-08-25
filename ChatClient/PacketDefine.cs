﻿using System;

namespace ChatClient {
    class PacketDefine {
        public const Int16 kPacketHeaderSize = 4;
        public const int kMaxUserIdLength = 30;
        public const int kMaxUserPwLength = 30;
        public const int kMaxRoomChatSize = 256;
        public const int kMaxPacketSize = 1024;
    }

    public enum PacketId : UInt16 {
        NTF_SYS_CLOSE = 3,

        LOGIN_REQ = 21,
        LOGIN_RES = 22,
        LOGOFF_REQ = 23,
        LOGOFF_RES = 24,

        LOBBY_LIST_REQ = 26,
        LOBBY_LIST_RES = 27,

        LOBBY_ENTER_REQ = 31,
        LOBBY_ENTER_RES = 32,
        LOBBY_ENTER_USER_INFO = 33,
        LOBBY_ENTER_USER_NTF = 34,

        LOBBY_LEAVE_REQ = 46,
        LOBBY_LEAVE_RES = 47,
        LOBBY_LEAVE_USER_NTF = 48,

        ROOM_ENTER_REQ = 61,
        ROOM_ENTER_RES = 62,
        ROOM_ENTER_USER_NTF = 63,

        ROOM_LEAVE_REQ = 66,
        ROOM_LEAVE_RES = 67,
        ROOM_LEAVE_USER_NTF = 68,

        ROOM_CHAT_REQ = 76,
        ROOM_CHAT_RES = 77,
        ROOM_CHAT_NOTIFY = 78
    }

    public enum ErrorCode : short
    {
        NONE = 0,

        UNASSIGNED_ERROR = 201,

        MAIN_INIT_NETWORK_INIT_FAIL = 206,

        USER_MGR_ID_DUPLICATION = 211,
        USER_MGR_MAX_USER_COUNT = 212,
        USER_MGR_INVALID_SESSION_INDEX = 213,
        USER_MGR_NOT_SET_USER = 214,
        USER_MGR_REMOVE_INVALID_SESSION = 221,

        LOGIN_ID_PW_MISMATCH = 223,
        LOBBY_LIST_INVALID_DOMAIN = 226,

        LOBBY_ENTER_INVALID_DOMAIN = 231,
        LOBBY_ENTER_INVALID_LOBBY_INDEX = 232,
        LOBBY_ENTER_USER_DUPLICATION = 234,
        LOBBY_ENTER_MAX_USER_COUNT = 235,
        LOBBY_ENTER_EMPTY_USER_LIST = 236,

        LOBBY_ROOM_LIST_INVALID_START_ROOM_INDEX = 241,
        LOBBY_ROOM_LIST_INVALID_DOMAIN = 242,
        LOBBY_ROOM_LIST_INVALID_LOBBY_INDEX = 243,

        LOBBY_USER_LIST_INVALID_DOMAIN = 251,
        LOBBY_USER_LIST_INVALID_LOBBY_INDEX = 252,
        LOBBY_USER_LIST_INVALID_START_USER_INDEX = 253,

        LOBBY_LEAVE_INVALID_DOMAIN = 261,
        LOBBY_LEAVE_INVALID_LOBBY_INDEX = 262,
        LOBBY_LEAVE_USER_INVALID = 263,

        ROOM_ENTER_INVALID_DOMAIN = 271,
        ROOM_ENTER_INVALID_LOBBY_INDEX = 272,
        ROOM_ENTER_INVALID_ROOM_INDEX = 273,
        ROOM_ALREADY_CREATED = 274,
        ROOM_NOT_CREATED = 275,
        ROOM_ENTER_MEMBER_FULL = 276,
        ROOM_ENTER_EMPTY_ROOM = 277,

        ROOM_LEAVE_INVALID_DOMAIN = 286,
        ROOM_LEAVE_INVALID_LOBBY_INDEX = 287,
        ROOM_LEAVE_INVALID_ROOM_INDEX = 288,
        ROOM_LEAVE_NOT_CREATED = 289,
        ROOM_LEAVE_NOT_MEMBER = 290,
        ROOM_CHAT_INVALID_DOMAIN = 296,
        ROOM_CHAT_INVALID_LOBBY_INDEX = 297,
        ROOM_CHAT_INVALID_ROOM_INDEX = 298,

        LOBBY_CHAT_INVALID_DOMAIN = 306,
        LOBBY_CHAT_INVALID_LOBBY_INDEX = 307,

        ROOM_MASTER_GAME_START_INVALID_DOMAIN = 401,
        ROOM_MASTER_GAME_START_INVALID_LOBBY_INDEX = 402,
        ROOM_MASTER_GAME_START_INVALID_ROOM_INDEX = 403,
        ROOM_MASTER_GAME_START_INVALID_MASTER = 404,
        ROOM_MASTER_GAME_START_INVALID_GAME_STATE = 405,
        ROOM_MASTER_GAME_START_INVALID_USER_COUNT = 406,
    }
}