import face_recognition
import os
import cv2
import numpy as np

import queue, threading, time

# bufferless VideoCapture
class VideoCapture:

  def __init__(self,name):
    self.cap = cv2.VideoCapture(name)
    self.q = queue.Queue()
    t = threading.Thread(target=self._reader)
    t.daemon = True
    t.start()

  # read frames as soon as they are available, keeping only most recent one
  def _reader(self):
    while True:
      ret, frame = self.cap.read()
      if not ret:
        break
      if not self.q.empty():
        try:
          self.q.get_nowait()   # discard previous (unprocessed) frame
        except queue.Empty:
          pass
      self.q.put(frame)

  def read(self):
    return self.q.get()

def save_file(vector,name):
    root = os.path.dirname(name)
    if(not os.path.exists(root)):
        os.mkdir(root)
    np.save(name,vector)


def getFaceEncoding(src):
    name = "output/2/" +os.path.basename(src).split(".")[0]
    image = face_recognition.load_image_file(src)
    face_locations = face_recognition.face_locations(image)
    face_encoding = face_recognition.face_encodings(image, face_locations)[0]
    save_file(face_encoding,name)
    return face_encoding

def theSamePerson(pic1):
    face_locations = face_recognition.face_locations(pic1)
    if face_recognition.face_encodings(pic1, face_locations):
        face_encoding = face_recognition.face_encodings(pic1, face_locations)[0]
        current_file = "output/2/pic1.npy"
        current_meber_encoding = np.load(current_file, encoding='bytes', allow_pickle=True)
        results = face_recognition.compare_faces([face_encoding],current_meber_encoding,tolerance=0.6)
        if (results == [True]):
            return 1
        
        # all_meber = os.listdir("output/2")
        # print(all_meber)
        # for meber in all_meber:
        #     print(meber)
        #     # current_name = meber.split(".")[0]
        #     current_file = "output/2"  + meber
        #     current_meber_encoding = np.load(current_file, encoding='bytes', allow_pickle=True)
        #     results = face_recognition.compare_faces([face_encoding],current_meber_encoding,tolerance=0.6)
        #     print("******")
        #     print(results)
        #     print("******")
        #     if (results == [True]):
        #         return 1
        return 0
    return 0

cap = VideoCapture(1)

def match():
    # file = "output/1/pic1.png"
    # getFaceEncoding(file)
    Is = 0
    frame = cap.read()
    Is = theSamePerson(frame)

    return Is