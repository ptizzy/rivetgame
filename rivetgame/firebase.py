import firebase_admin
from firebase_admin import credentials
from firebase_admin import db

# Initialize the app with a service account, granting admin privileges
firebase_admin.initialize_app(
    credentials.Certificate("ideaburgercode-9fdaa9bf21f0.json"),
    {
        'databaseURL': 'https://ideaburgercode.firebaseio.com/'
    })

def push_data_log(data):
    try:
        if len(data) == 0:
            return
        ref = db.reference('data/' + len(data))
        ref.push({i: v for i, v in enumerate(data)})
    except Exception as e:
        print("Failed to firebase" + e)


def push_text_log(text):
    try:
        ref = db.reference('logs')
        ref.push(text)
    except Exception as e:
        print("Failed to firebase" + e)


if __name__ == '__main__':
    push_data_log([1, 3, 45, 8.4])
    push_text_log("This is a test")