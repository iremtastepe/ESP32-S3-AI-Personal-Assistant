"""ESP32 Cat Assistant: mevcut cat_sleep PNG/C verisinden nefes alma kareleri.
"""
from __future__ import annotations
import math
import re
import sys
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    raise SystemExit('Pillow bulunamadi. Bir kez: python -m pip install Pillow')

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / 'components' / 'cat_assets' / 'include'
PREVIEW = ROOT / 'tools' / 'cat_breath_preview'


def from_c(path: Path) -> Image.Image:
    """LVGL 9 tarafindan uretilmis yaygin cat_sleep.c bicimlerini acar."""
    s = path.read_text(encoding='utf-8-sig', errors='replace')
    desc = re.search(r'\b(?:const\s+)?lv_image_dsc_t\s+cat_sleep\s*=\s*\{(.*?)\};', s, re.S)
    scope = desc.group(1) if desc else s
    def val(pattern: str) -> str:
        m = re.search(pattern, scope)
        if not m:
            raise ValueError(f'cat_sleep.c icinde alan bulunamadi: {pattern}')
        return m.group(1)
    w = int(val(r'\.w\s*=\s*(\d+)'))
    h = int(val(r'\.h\s*=\s*(\d+)'))
    cf = val(r'\.cf\s*=\s*(LV_COLOR_FORMAT_[A-Z0-9_]+)')
    arr = re.search(r'\bcat_sleep_map\s*\[[^]]*\]\s*(?:LV_ATTRIBUTE_MEM_ALIGN\s*)?=\s*\{(.*?)\};', s, re.S)
    if not arr:
        # Bazi donusturuculer farkli veri degiskeni adi kullaniyor.
        p = re.search(r'\.data\s*=\s*(\w+)', scope)
        if p:
            arr = re.search(r'\b' + re.escape(p.group(1)) + r'\s*\[[^]]*\]\s*(?:LV_ATTRIBUTE_MEM_ALIGN\s*)?=\s*\{(.*?)\};', s, re.S)
    if not arr:
        raise ValueError('cat_sleep.c resim verisi (byte dizisi) bulunamadi.')
    body = re.sub(r'/\*.*?\*/|//[^\n]*', '', arr.group(1), flags=re.S)
    vals = [int(v, 0) for v in re.findall(r'0x[0-9A-Fa-f]+|\b\d+\b', body)]
    data = bytes(vals)
    n = w * h
    print(f'Kaynak: {path.name}, {w}x{h}, {cf}, {len(data)} bayt')
    if cf == 'LV_COLOR_FORMAT_ARGB8888':
        if len(data) < n*4: raise ValueError('ARGB8888 veri uzunlugu yetersiz')
        return Image.frombytes('RGBA', (w,h), data[:n*4], 'raw', 'BGRA')
    if cf == 'LV_COLOR_FORMAT_XRGB8888':
        if len(data) < n*4: raise ValueError('XRGB8888 veri uzunlugu yetersiz')
        raw = bytearray(data[:n*4]); raw[3::4] = bytes([255])*n
        return Image.frombytes('RGBA', (w,h), bytes(raw), 'raw', 'BGRA')
    if cf in ('LV_COLOR_FORMAT_RGB565A8', 'LV_COLOR_FORMAT_RGB565'):
        if len(data) < n*2: raise ValueError('RGB565 veri uzunlugu yetersiz')
        if cf.endswith('A8') and len(data) < n*3: raise ValueError('RGB565A8 alfa verisi eksik')
        result = bytearray(n*4)
        for i in range(n):
            c = data[i*2] | (data[i*2+1]<<8)
            result[i*4] = ((c>>11)&31)*255//31
            result[i*4+1] = ((c>>5)&63)*255//63
            result[i*4+2] = (c&31)*255//31
            result[i*4+3] = data[n*2+i] if cf.endswith('A8') else 255
        return Image.frombytes('RGBA',(w,h),bytes(result))
    if cf == 'LV_COLOR_FORMAT_RGB888':
        if len(data)< n*3: raise ValueError('RGB888 veri uzunlugu yetersiz')
        return Image.frombytes('RGB',(w,h),data[:n*3], 'raw','BGR').convert('RGBA')
    raise ValueError(f'Otomatik okuma icin desteklenmeyen format: {cf}. PNG dosyasini kullan.')


def find_source() -> Image.Image:
    # MUMKUNSE ilk olarak birebir orijinal PNG kullanilir.
    for f in (ROOT/'cat_sleep.png', ROOT/'main'/'cat_sleep.png', ROOT/'assets'/'cat_sleep.png'):
        if f.is_file():
            print('Orijinal PNG kullaniliyor:', f)
            return Image.open(f).convert('RGBA')
    for f in (OUT/'cat_sleep.c', ROOT/'cat_sleep.c'):
        if f.is_file():
            return from_c(f)
    raise SystemExit('Kaynak bulunamadi: cat_sleep.png veya main/cat_sleep.c gerekli.')


def smoothstep(v: float) -> float:
    v = max(0.0, min(1.0, v))
    return v*v*(3.0-2.0*v)


def breath_frame(src: Image.Image, amplitude: float) -> Image.Image:
    """Yalnizca sirt/govde ustunu hafif kaldirir; kafa, patiler, minder sabit."""
    w,h = src.size
    px = src.load()
    dst = Image.new('RGBA', (w,h))
    out = dst.load()
    # Referans cizim 208x144. Buyuklukler otomatik olceklenir.
    sx, sy = w/208.0, h/144.0
    for y in range(h):
        yy = y/sy
        # Govdenin alti ve minder etkilenmeyecek.
        wy = smoothstep((yy-23.0)/13.0) * smoothstep((96.0-yy)/28.0)
        for x in range(w):
            xx = x/sx
            # Kedi basi solda, govde sagda.
            wx = smoothstep((xx-103.0)/22.0) * smoothstep((198.0-xx)/23.0)
            offset = amplitude * sy * wx * wy
            srcy = min(h-1, max(0, y+offset))  # yukari dogru kucuk genisleme
            lo = int(srcy); hi = min(h-1,lo+1); frac=srcy-lo
            if frac < .001:
                out[x,y]=px[x,lo]
            else:
                a=px[x,lo]; b=px[x,hi]
                out[x,y]=tuple(round((1-frac)*a[c]+frac*b[c]) for c in range(4))
    return dst


def to_bgra(img: Image.Image) -> bytes:
    # LVGL ARGB8888: ESP32 little-endian bellekte BGRA bayt sirasi.
    return img.convert('RGBA').tobytes('raw','BGRA')


def create_cpp_header(frames: list[Image.Image]) -> None:
    w,h=frames[0].size
    target = OUT/'cat_breath_frames.h'
    with target.open('w',encoding='utf-8') as f:
        f.write('// OTOMATIK OLUSTURULDU: make_breath.py. Elle duzenlemeyin.\n')
        f.write('#pragma once\n#include "lvgl.h"\n\n')
        f.write('// Flash belleginde duran, 4 saydam ARGB8888 animasyon karesi.\n')
        for i,frame in enumerate(frames):
            data=to_bgra(frame)
            f.write(f'static const uint8_t cat_breath_pixels_{i}[] LV_ATTRIBUTE_MEM_ALIGN = {{\n')
            for p in range(0,len(data),24):
                f.write('    '+', '.join(f'0x{v:02x}' for v in data[p:p+24])+',\n')
            f.write('};\n\n')
        f.write(f'''static inline void cat_breath_setup_frame(lv_image_dsc_t *dsc,
                                                  const uint8_t *data,
                                                  uint32_t data_size) {{
    *dsc = {{}};
    dsc->header.magic = LV_IMAGE_HEADER_MAGIC;
    dsc->header.cf = LV_COLOR_FORMAT_ARGB8888;
    dsc->header.w = {w};
    dsc->header.h = {h};
    dsc->header.stride = {w*4};
    dsc->data_size = data_size;
    dsc->data = data;
}}
''')
    print('Olusturuldu:', target)


def main():
    OUT.mkdir(exist_ok=True)
    PREVIEW.mkdir(exist_ok=True)
    src=find_source()
    if src.size[0] < 100 or src.size[1] < 80:
        print('UYARI: Gorsel boyutu beklenenden kucuk; animasyon alani kayabilir.')
    # 0->1->2->1, yumusak dongu.
    frames=[breath_frame(src,a) for a in (0.0, 2.5, 4.5, 2.5)]
    for i,fr in enumerate(frames):
        fr.save(PREVIEW/f'cat_breath_{i}.png')
    comps=[]
    for fr in frames:
        bg=Image.new('RGBA',fr.size,'#F5F0E8');bg.alpha_composite(fr)
        comps.append(bg.convert('RGB'))
    comps[0].save(PREVIEW/'cat_breath_preview.gif', save_all=True,
                  append_images=comps[1:],duration=[800,800,800,800],loop=0)
    create_cpp_header(frames)
    print('ONIZLEME:',PREVIEW/'cat_breath_preview.gif')
    print('Tamam! main.cpp henüz DEGISTIRILMEDI; once GIF onizlemesini kontrol et.')

if __name__ == '__main__':
    try:
        main()
    except Exception as exc:
        raise SystemExit(f'Hazirlama hatasi: {exc}')
