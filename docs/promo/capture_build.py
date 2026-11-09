#!/usr/bin/env python3
import json, base64, os, time, subprocess, urllib.request, shutil
from websocket import create_connection
import imageio_ffmpeg
CH="/Applications/Google Chrome.app/Contents/MacOS/Google Chrome"
HERE=os.path.dirname(os.path.abspath(__file__)); os.chdir(HERE)
FR=os.path.join(HERE,"frames")
if os.path.isdir(FR): shutil.rmtree(FR)
os.makedirs(FR)
proc=subprocess.Popen([CH,"--headless=new","--disable-gpu","--hide-scrollbars",
  "--force-device-scale-factor=1","--window-size=1920,1080","--remote-debugging-port=9222",
  "--remote-allow-origins=*","--user-data-dir=/tmp/chrome-cdp-coqui","about:blank"],
  stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
ws_url=None
for _ in range(120):
    try:
        d=json.load(urllib.request.urlopen("http://localhost:9222/json"))
        pages=[t for t in d if t.get("type")=="page"]
        if pages: ws_url=pages[0]["webSocketDebuggerUrl"]; break
    except Exception: pass
    time.sleep(0.1)
assert ws_url, "no devtools"
ws=create_connection(ws_url, max_size=None); mid=0
def cmd(method, params=None):
    global mid; mid+=1; i=mid
    ws.send(json.dumps({"id":i,"method":method,"params":params or {}}))
    while True:
        m=json.loads(ws.recv())
        if m.get("id")==i: return m.get("result",{})
cmd("Page.enable"); cmd("Runtime.enable")
cmd("Emulation.setDeviceMetricsOverride",{"width":1920,"height":1080,"deviceScaleFactor":1,"mobile":False})
cmd("Page.navigate",{"url":"file://"+os.path.join(HERE,"anim.html")})
time.sleep(1.6)
cmd("Runtime.evaluate",{"expression":"document.fonts.ready","awaitPromise":True})
time.sleep(0.7)
N=300
for k in range(N):
    t=k/30.0
    cmd("Runtime.evaluate",{"expression":"renderAt(%f)"%t})
    r=cmd("Page.captureScreenshot",{"format":"png","clip":{"x":0,"y":0,"width":1920,"height":1080,"scale":1}})
    open(os.path.join(FR,"f%04d.png"%k),"wb").write(base64.b64decode(r["data"]))
ws.close(); proc.terminate()
print("captured",N,"frames")
FF=imageio_ffmpeg.get_ffmpeg_exe()
r=subprocess.run([FF,"-y","-framerate","30","-i",os.path.join(FR,"f%04d.png"),"-i","band.mp3",
  "-filter_complex","[1:a]atrim=0:10,asetpts=PTS-STARTPTS,afade=t=in:st=0:d=0.3,afade=t=out:st=9:d=1[a]",
  "-map","0:v","-map","[a]","-c:v","libx264","-preset","medium","-crf","18","-pix_fmt","yuv420p","-r","30",
  "-c:a","aac","-b:a","160k","-shortest","coqui-promo-10s-anim.mp4"],capture_output=True,text=True)
if r.returncode: print("FFMPEG FAIL\n",r.stderr[-1500:]); raise SystemExit(1)
print("DONE coqui-promo-10s-anim.mp4")
