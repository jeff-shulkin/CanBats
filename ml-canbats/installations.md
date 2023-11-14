Need pip for tensorflow
```
sudo apt install pip
sudo apt install ffmpeg
```

Need to break system packages
```
pip install --upgrade pip --break-system-packages
# pip install tensorflow --break-system-packages
pip install Pillow --break-system-packages
pip install librosa --break-system-packages
pip install matplotlib --break-system-packages
```

Use runtime instead (todo: check with fresh install with runtime only)
```
# pip uninstall tensorflow --break-system-packages
pip install tflite-runtime --break-system-packages
```