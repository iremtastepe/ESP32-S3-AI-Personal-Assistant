from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

# Windows sistem fontu kullanilir; kullaniciya font dosyasi dagitilmaz.
font_dir = Path('C:/Windows/Fonts')
font_path = next((p for p in (font_dir / 'arial.ttf', font_dir / 'segoeui.ttf', font_dir / 'tahoma.ttf') if p.exists()), None)
if font_path is None:
    raise SystemExit('Windows Fonts icinde Arial/Segoe UI/Tahoma bulunamadi.')

codes = list(range(0x20, 0x7F))
extra = sorted({ord(ch) for ch in '°ÇÖÜçöüĞğİışŞş♡✓↻' if ord(ch) not in codes})
codes += extra


def render_glyphs(size):
    ft = ImageFont.truetype(str(font_path), size)
    asc, desc = ft.getmetrics()
    glyphs, raw = [(0, 0, 0, 0, 0, 0)], bytearray()
    # Each descriptor: bitmap_index, adv_w (8.4), box_w, box_h, ofs_x, ofs_y
    for cp in codes:
        ch = chr(cp)
        l, t, r, b = ft.getbbox(ch, anchor='ls')
        w, h = max(0, r - l), max(0, b - t)
        adv = max(0, round(ft.getlength(ch) * 16))
        start = len(raw)
        if w and h:
            img = Image.new('L', (w, h), 0)
            painter = ImageDraw.Draw(img)
            painter.text((-l, -t), ch, font=ft, fill=255, anchor='ls')
            px = [min(15, (v + 8) // 17) for v in img.getdata()]
            for i in range(0, len(px), 2):
                raw.append((px[i] << 4) | (px[i + 1] if i + 1 < len(px) else 0))
        glyphs.append((start, adv, w, h, l, -b))
    return raw, glyphs, asc + desc, desc


def to_c_array(nums, indent='    ', per_line=16, h=True):
    out = []
    for i in range(0, len(nums), per_line):
        items = nums[i:i+per_line]
        out.append(indent + ', '.join(f'0x{v:02X}' if h else str(v) for v in items) + ',')
    return '\n'.join(out)


def emit_font(size):
    bitmap, glyphs, height, baseline = render_glyphs(size)
    tag = f'tr_font_{size}'
    desc = '\n'.join('    {%d, %d, %d, %d, %d, %d},' % entry for entry in glyphs)
    extras = [cp - extra[0] for cp in extra]
    text = f'''// Auto-generated using a locally installed Windows system font.
static const uint8_t {tag}_bitmap[] = {{
{to_c_array(bitmap)}
}};

static const lv_font_fmt_txt_glyph_dsc_t {tag}_glyphs[] = {{
{desc}
}};

static const uint16_t {tag}_extra[] = {{
{to_c_array(extras, h=False)}
}};

static const lv_font_fmt_txt_cmap_t {tag}_cmaps[] = {{
    {{0x20, 0x5F, 1, nullptr, nullptr, 0, LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY}},
    {{0x{extra[0]:X}, 0x{extra[-1] - extra[0] + 1:X}, 96, {tag}_extra, nullptr, {len(extra)}, LV_FONT_FMT_TXT_CMAP_SPARSE_TINY}},
}};

static const lv_font_t {tag} = [] {{
    static lv_font_fmt_txt_dsc_t dsc = {{}};
    dsc.glyph_bitmap = {tag}_bitmap;
    dsc.glyph_dsc = {tag}_glyphs;
    dsc.cmaps = {tag}_cmaps;
    dsc.cmap_num = 2;
    dsc.bpp = 4;
    dsc.bitmap_format = LV_FONT_FMT_TXT_PLAIN;
    lv_font_t f = {{}};
    f.get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt;
    f.get_glyph_bitmap = lv_font_get_bitmap_fmt_txt;
    f.line_height = {height};
    f.base_line = {baseline};
    f.subpx = LV_FONT_SUBPX_NONE;
    f.underline_position = -1;
    f.underline_thickness = 1;
    f.dsc = &dsc;
#if LV_VERSION_CHECK(9, 3, 0)
    f.static_bitmap = 1;
#endif
    return f;
}}();
'''
    return text

source = '#pragma once\n#include "lvgl.h"\n\n' + emit_font(12) + '\n' + emit_font(14)
Path(__file__).with_name('tr_font.h').write_text(source, encoding='utf-8')
print('tr_font.h olusturuldu.')
