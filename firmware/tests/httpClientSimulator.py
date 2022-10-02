import requests

headers = {'Content-type': 'text/plain',
            'filename'   : 'test_rx.txt'
          }

res = requests.get('http://192.168.4.1:80/getFileContent', headers=headers)

print(res.text)