from config.logger import setup_logging
import json
import asyncio
import time
from core.utils.util import remove_punctuation_and_length, get_string_no_punctuation_or_emoji


TAG = __name__
logger = setup_logging()


emoji_map = {
    'neutral': '😶',
    'happy': '🙂',
    'laughing': '😆',
    'funny': '😂',
    'sad': '😔',
    'angry': '😠',
    'crying': '😭',
    'loving': '😍',
    'embarrassed': '😳',
    'surprised': '😲',
    'shocked': '😱',
    'thinking': '🤔',
    'winking': '😉',
    'cool': '😎',
    'relaxed': '😌',
    'delicious': '🤤',
    'kissy': '😘',
    'confident': '😏',
    'sleepy': '😴',
    'silly': '😜',
    'confused': '🙄'
}

def analyze_emotion(text):
    """
    分析文本情感并返回对应的emoji名称（支持中英文）
    """
    if not text or not isinstance(text, str):
        return 'neutral'

    original_text = text
    text = text.lower().strip()

    # 检查是否包含现有emoji
    for emotion, emoji in emoji_map.items():
        if emoji in original_text:
            return emotion

    # 标点符号分析
    has_exclamation = '!' in original_text or '！' in original_text
    has_question = '?' in original_text or '？' in original_text
    has_ellipsis = '...' in original_text or '…' in original_text

    # 定义情感关键词映射（中英文扩展版）
    emotion_keywords = {
        'happy': ['开心', '高兴', '快乐', '愉快', '幸福', '满意', '棒', '好', '不错', '完美', '棒极了', '太好了',
                  '好呀', '好的', 'happy', 'joy', 'great', 'good', 'nice', 'awesome', 'fantastic', 'wonderful'],
        'laughing': ['哈哈', '哈哈哈', '呵呵', '嘿嘿', '嘻嘻', '笑死', '太好笑了', '笑死我了', 'lol', 'lmao', 'haha',
                     'hahaha', 'hehe', 'rofl', 'funny', 'laugh'],
        'funny': ['搞笑', '滑稽', '逗', '幽默', '笑点', '段子', '笑话', '太逗了', 'hilarious', 'joke', 'comedy'],
        'sad': ['伤心', '难过', '悲哀', '悲伤', '忧郁', '郁闷', '沮丧', '失望', '想哭', '难受', '不开心', '唉', '呜呜',
                'sad', 'upset', 'unhappy', 'depressed', 'sorrow', 'gloomy'],
        'angry': ['生气', '愤怒', '气死', '讨厌', '烦人', '可恶', '烦死了', '恼火', '暴躁', '火大', '愤怒', '气炸了',
                  'angry', 'mad', 'annoyed', 'furious', 'pissed', 'hate'],
        'crying': ['哭泣', '泪流', '大哭', '伤心欲绝', '泪目', '流泪', '哭死', '哭晕', '想哭', '泪崩',
                   'cry', 'crying', 'tears', 'sob', 'weep'],
        'loving': ['爱你', '喜欢', '爱', '亲爱的', '宝贝', '么么哒', '抱抱', '想你', '思念', '最爱', '亲亲', '喜欢你',
                   'love', 'like', 'adore', 'darling', 'sweetie', 'honey', 'miss you', 'heart'],
        'embarrassed': ['尴尬', '不好意思', '害羞', '脸红', '难为情', '社死', '丢脸', '出丑',
                        'embarrassed', 'awkward', 'shy', 'blush'],
        'surprised': ['惊讶', '吃惊', '天啊', '哇塞', '哇', '居然', '竟然', '没想到', '出乎意料',
                      'surprise', 'wow', 'omg', 'oh my god', 'amazing', 'unbelievable'],
        'shocked': ['震惊', '吓到', '惊呆了', '不敢相信', '震撼', '吓死', '恐怖', '害怕', '吓人',
                    'shocked', 'shocking', 'scared', 'frightened', 'terrified', 'horror'],
        'thinking': ['思考', '考虑', '想一下', '琢磨', '沉思', '冥想', '想', '思考中', '在想',
                      'think', 'thinking', 'consider', 'ponder', 'meditate'],
        'winking': ['调皮', '眨眼', '你懂的', '坏笑', '邪恶', '奸笑', '使眼色',
                    'wink', 'teasing', 'naughty', 'mischievous'],
        'cool': ['酷', '帅', '厉害', '棒极了', '真棒', '牛逼', '强', '优秀', '杰出', '出色', '完美',
                 'cool', 'awesome', 'amazing', 'great', 'impressive', 'perfect'],
        'relaxed': ['放松', '舒服', '惬意', '悠闲', '轻松', '舒适', '安逸', '自在',
                     'relax', 'relaxed', 'comfortable', 'cozy', 'chill', 'peaceful'],
        'delicious': ['好吃', '美味', '香', '馋', '可口', '香甜', '大餐', '大快朵颐', '流口水', '垂涎',
                      'delicious', 'yummy', 'tasty', 'yum', 'appetizing', 'mouthwatering'],
        'kissy': ['亲亲', '么么', '吻', 'mua', 'muah', '亲一下', '飞吻',
                  'kiss', 'xoxo', 'hug', 'muah', 'smooch'],
        'confident': ['自信', '肯定', '确定', '毫无疑问', '当然', '必须的', '毫无疑问', '确信', '坚信',
                      'confident', 'sure', 'certain', 'definitely', 'positive'],
        'sleepy': ['困', '睡觉', '晚安', '想睡', '好累', '疲惫', '疲倦', '困了', '想休息', '睡意',
                   'sleep', 'sleepy', 'tired', 'exhausted', 'bedtime', 'good night'],
        'silly': ['傻', '笨', '呆', '憨', '蠢', '二', '憨憨', '傻乎乎', '呆萌',
                  'silly', 'stupid', 'dumb', 'foolish', 'goofy', 'ridiculous'],
        'confused': ['疑惑', '不明白', '不懂', '困惑', '疑问', '为什么', '怎么回事', '啥意思', '不清楚',
                     'confused', 'puzzled', 'doubt', 'question', 'what', 'why', 'how']
    }

    # 特殊句型判断（中英文）
    # 赞美他人
    if any(phrase in text for phrase in
           ['你真', '你好', '您真', '你真棒', '你好厉害', '你太强了', '你真好', '你真聪明',
            'you are', 'you\'re', 'you look', 'you seem', 'so smart', 'so kind']):
        return 'loving'
    # 自我赞美
    if any(phrase in text for phrase in ['我真', '我最', '我太棒了', '我厉害', '我聪明', '我优秀',
                                        'i am', 'i\'m', 'i feel', 'so good', 'so happy']):
        return 'cool'
    # 晚安/睡觉相关
    if any(phrase in text for phrase in ['睡觉', '晚安', '睡了', '好梦', '休息了', '去睡了',
                                         'sleep', 'good night', 'bedtime', 'go to bed']):
        return 'sleepy'
    # 疑问句
    if has_question and not has_exclamation:
        return 'thinking'
    # 强烈情感（感叹号）
    if has_exclamation and not has_question:
        # 检查是否是积极内容
        positive_words = emotion_keywords['happy'] + emotion_keywords['laughing'] + emotion_keywords['cool']
        if any(word in text for word in positive_words):
            return 'laughing'
        # 检查是否是消极内容
        negative_words = emotion_keywords['angry'] + emotion_keywords['sad'] + emotion_keywords['crying']
        if any(word in text for word in negative_words):
            return 'angry'
        return 'surprised'
    # 省略号（表示犹豫或思考）
    if has_ellipsis:
        return 'thinking'

    # 关键词匹配（带权重）
    emotion_scores = {emotion: 0 for emotion in emoji_map.keys()}

    # 给匹配到的关键词加分
    for emotion, keywords in emotion_keywords.items():
        for keyword in keywords:
            if keyword in text:
                emotion_scores[emotion] += 1

    # 给长文本中的重复关键词额外加分
    if len(text) > 20:  # 长文本
        for emotion, keywords in emotion_keywords.items():
            for keyword in keywords:
                emotion_scores[emotion] += text.count(keyword) * 0.5

    # 根据分数选择最可能的情感
    max_score = max(emotion_scores.values())
    if max_score == 0:
        return 'happy'  # 默认

    # 可能有多个情感同分，根据上下文选择最合适的
    top_emotions = [e for e, s in emotion_scores.items() if s == max_score]

    # 如果多个情感同分，使用以下优先级
    priority_order = [
        'laughing', 'crying', 'angry', 'surprised', 'shocked',  # 强烈情感优先
        'loving', 'happy', 'funny', 'cool',  # 积极情感
        'sad', 'embarrassed', 'confused',  # 消极情感
        'thinking', 'winking', 'relaxed',  # 中性情感
        'delicious', 'kissy', 'confident', 'sleepy', 'silly'  # 特殊场景
    ]

    for emotion in priority_order:
        if emotion in top_emotions:
            return emotion

    return top_emotions[0]  # 如果都不在优先级列表里，返回第一个


# async def sendAudioMessage(conn, audios, text, text_index=0):
#     # 发送句子开始消息
#     if text_index == conn.tts_first_text_index:
#         logger.bind(tag=TAG).info(f"发送第一段语音: {text}")
#     await send_tts_message(conn, "sentence_start", text)

#     # 流控参数优化
#     original_frame_duration = 60  # 原始帧时长（毫秒）
#     adjusted_frame_duration = int(original_frame_duration * 0.8)  # 缩短20%
#     total_frames = len(audios)  # 获取总帧数
#     compensation = total_frames * (original_frame_duration - adjusted_frame_duration) / 1000  # 补偿时间（秒）

#     start_time = time.perf_counter()
#     play_position = 0  # 已播放时长（毫秒）

#     for opus_packet in audios:
#         if conn.client_abort:
#             return

#         # 计算带加速因子的预期时间
#         expected_time = start_time + (play_position / 1000)
#         current_time = time.perf_counter()

#         # 流控等待（使用加速后的帧时长）
#         delay = expected_time - current_time
#         if delay > 0:
#             await asyncio.sleep(delay)

#         await conn.websocket.send(opus_packet)
#         play_position += adjusted_frame_duration  # 使用调整后的帧时长

#     # 补偿因加速损失的时长
#     if compensation > 0:
#         await asyncio.sleep(compensation)

#     await send_tts_message(conn, "sentence_end", text)

#     # 发送结束消息（如果是最后一个文本）
#     if conn.llm_finish_task and text_index == conn.tts_last_text_index:
#         await send_tts_message(conn, 'stop', None)
#         if conn.close_after_chat:
#             await conn.close()

# async def send_tts_message(conn, state, text=None):
#     """发送 TTS 状态消息"""
#     message = {
#         "type": "tts",
#         "state": state,
#         "session_id": conn.session_id
#     }
#     if text is not None:
#         message["text"] = text

#     await conn.websocket.send(json.dumps(message))
#     if state == "stop":
#         conn.clearSpeakStatus()


# async def send_stt_message(conn, text):
#     """发送 STT 状态消息"""
#     stt_text = get_string_no_punctuation_or_emoji(text)
#     await conn.websocket.send(json.dumps({
#         "type": "stt",
#         "text": stt_text,
#         "session_id": conn.session_id}
#     ))
#     await conn.websocket.send(
#         json.dumps({
#             "type": "llm",
#             "text": "😊",
#             "emotion": "happy",
#             "session_id": conn.session_id}
#         ))
#     await send_tts_message(conn, "start")


async def sendAudioMessage(conn, audios, text, text_index=0):
    # 发送句子开始消息
    if text is not None:
        emotion = analyze_emotion(text)
        emoji = emoji_map.get(emotion, "🙂")  # 默认使用笑脸
        await conn.websocket.send(
            json.dumps(
                {
                    "type": "llm",
                    "text": emoji,
                    "emotion": emotion,
                    "session_id": conn.session_id,
                }
            )
        )

    if text_index == conn.tts_first_text_index:
        logger.bind(tag=TAG).info(f"发送第一段语音: {text}")
    await send_tts_message(conn, "sentence_start", text)

    is_first_audio = (text_index == conn.tts_first_text_index)
    await sendAudio(conn, audios, pre_buffer=is_first_audio)

    await send_tts_message(conn, "sentence_end", text)

    # 发送结束消息（如果是最后一个文本）
    if conn.llm_finish_task and text_index == conn.tts_last_text_index:
        await send_tts_message(conn, "stop", None)
        if conn.close_after_chat:
            await conn.close()


# 播放音频
async def sendAudio(conn, audios, pre_buffer=True):
    # 流控参数优化
    frame_duration = 60  # 帧时长（毫秒），匹配 Opus 编码
    start_time = time.perf_counter()
    play_position = 0
    last_reset_time = time.perf_counter()  # 记录最后的重置时间

    # 仅当第一句话时执行预缓冲
    if pre_buffer:
        pre_buffer_frames = min(3, len(audios))
        for i in range(pre_buffer_frames):
            await conn.websocket.send(audios[i])
        remaining_audios = audios[pre_buffer_frames:]
    else:
        remaining_audios = audios

    # 播放剩余音频帧
    for opus_packet in remaining_audios:
        if conn.client_abort:
            return

        # 每分钟重置一次计时器
        if time.perf_counter() - last_reset_time > 60:
            await conn.reset_timeout()
            last_reset_time = time.perf_counter()

        # 计算预期发送时间
        expected_time = start_time + (play_position / 1000)
        current_time = time.perf_counter()
        delay = expected_time - current_time
        if delay > 0:
            await asyncio.sleep(delay)

        await conn.websocket.send(opus_packet)

        play_position += frame_duration


async def send_tts_message(conn, state, text=None):
    """发送 TTS 状态消息"""
    message = {"type": "tts", "state": state, "session_id": conn.session_id}
    if text is not None:
        message["text"] = text

    # TTS播放结束
    if state == "stop":
        # 播放提示音
        tts_notify = conn.config.get("enable_stop_tts_notify", False)
        if tts_notify:
            stop_tts_notify_voice = conn.config.get(
                "stop_tts_notify_voice", "config/assets/tts_notify.mp3"
            )
            audios, duration = conn.tts.audio_to_opus_data(stop_tts_notify_voice)
            await sendAudio(conn, audios)
        # 清除服务端讲话状态
        conn.clearSpeakStatus()

    # 发送消息到客户端
    await conn.websocket.send(json.dumps(message))


async def send_stt_message(conn, text):
    """发送 STT 状态消息"""
    stt_text = get_string_no_punctuation_or_emoji(text)
    await conn.websocket.send(
        json.dumps({"type": "stt", "text": stt_text, "session_id": conn.session_id})
    )
    await send_tts_message(conn, "start")

