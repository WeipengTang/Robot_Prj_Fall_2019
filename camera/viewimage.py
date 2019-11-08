import PIL
import tkinter
from PIL import Image
from PIL import ImageTk


def update_image():
	global tkimg1
	tkimg1 = ImageTk.PhotoImage(Image.open('/home/a3154-18/Documents/Robot_Prj_Fall_2019/camera/image.png'))
	label.config(image=tkimg1)
	label.after(1000, update_image)
	print("updated")

w = tkinter.Tk()
im = Image.open('/home/a3154-18/Documents/Robot_Prj_Fall_2019/camera/image.png')
tkimg1 = ImageTk.PhotoImage(im)
label = tkinter.Label(w, image=tkimg1)
print("Loaded")
label.pack()
w.after(1000, update_image)
w.mainloop()
