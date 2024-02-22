import telebot
import flask
import time
from flask import render_template, request
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.backends import default_backend
from base64 import b64encode, b64decode


from supabase_py import create_client

supabase_url = 'https://wuqffzjavqxkzvabwnvz.supabase.co'
supabase_key = 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Ind1cWZmemphdnF4a3p2YWJ3bnZ6Iiwicm9sZSI6InNlcnZpY2Vfcm9sZSIsImlhdCI6MTcwODUwMDY3MCwiZXhwIjoyMDI0MDc2NjcwfQ.axfgDKqajY8xfm4AH2UarSP-iv-YIh64vNLWvzuvalU'
client = create_client(supabase_url, supabase_key)


key = b'6f3a825f7b3ed17c529e0be6e0441d9a'

token = '6822053828:AAHxDWES7AWJjRK91EawGS6k6IRvHfyy9PU'
bot = telebot.TeleBot(token, threaded=False)

WEBHOOK_HOST = 'teleapigps.pythonanywhere.com'
WEBHOOK_URL_BASE = "https://%s" % (WEBHOOK_HOST)
WEBHOOK_URL_PATH = "/%s/" % (token)

encrypted_data = ""
decrypted_data = ""

# def decrypt_aes(encrypted_text, key):
#     # Base64 decode the encrypted text
#     encrypted_data = b64decode(encrypted_text)

#     # Extract IV from the encrypted data
#     iv = encrypted_data[:16]  # IV should be 16 bytes for AES

#     # Initialize AES cipher with CBC mode
#     cipher = Cipher(algorithms.AES(key), modes.CBC(iv), backend=default_backend())
#     decryptor = cipher.decryptor()

#     # Decrypt the data
#     decrypted_data = decryptor.update(encrypted_data[16:]) + decryptor.finalize()

#     # Unpad the decrypted data
#     unpadded_data = decrypted_data[:-decrypted_data[-1]]

#     return unpadded_data.decode()


# decrypted_message = decrypt_aes(encrypted_data.encode(), key)


app = flask.Flask(__name__)

@app.route('/')
def koordinat():
    table_name = 'temperature'
    tempData = client.table(table_name).select().execute()
    keyLog = 'keyLog'
    keyLogData = client.table(keyLog).select().execute()
    dispp = render_template('Beranda.html',title='Beranda Smarthome Fakhrul',tempData=tempData, keyLogData = keyLogData)
    return dispp
@app.route('/temp', methods=['POST'])
def temperature():
    data = request.get_json()
    global humi, tempf, tempc, encrypted_data
    if data:
        humi = data.get('humi')
        tempf = data.get('tempf')
        tempc = data.get('tempc')
        # encrypted_data = flask.request.args.get("uid")
        new_record = {'temp_f': tempf, 'temp_c': tempc, 'humi' : humi}
        response = client.from_('temperature').insert(new_record).execute()
        if 'error' in response:
            print('Operation failed:', response['error'])
        else:
            print('Operation successful')
        return response
@app.route('/key', methods=['POST'])
def uidDetection():
    data = request.get_json()
    global uid, key
    if data:
        key = data.get('key')
        uid = data.get('uid')

        new_record = {'uid': uid}
        response = client.from_('keyLog').insert(new_record).execute()
        if 'error' in response:
            print('Operation failed:', response['error'])
        else:
            print('Operation successful')
        return response
# Process webhook calls
@app.route(WEBHOOK_URL_PATH, methods=['POST'])
def webhook():
    if flask.request.headers.get('content-type') == 'application/json':
        json_string = flask.request.get_data().decode('utf-8')
        update = telebot.types.Update.de_json(json_string)
        bot.process_new_updates([update])
        return ''
    else:
        flask.abort(403)

@bot.message_handler(commands=['start', 'help'])
def handle_start_help(message):
    bot.send_message(message.chat.id, text="Gunakan /status untuk melihat lokasi.")

@bot.message_handler(commands=['status'])
def handle_start_help(message):
    bot.send_message(message.chat.id, text=f"humiitude:{humi}\ntempf:{tempf}\ntempcitude:{tempc}\n\n")

bot.remove_webhook()
time.sleep(0.1)
bot.set_webhook(url=WEBHOOK_URL_BASE + WEBHOOK_URL_PATH)
