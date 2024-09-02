# Gamemon

Simple serial protocol for dumping framebuffers to an adminstrative monitor device, for game kiosks.

The main use case is presenting my games at conventions.
The games run on disparate hardware. Usually a MacBook, Raspberry Pi, or Atari VCS.
I want tiny monitors that work reliably and connect over USB, that can point back toward me to show what the user is up to.
If we can also program the device's buttons to do something useful, that would be great too.

Initially I'm going to use the TinyCircuits TinyArcade as monitors.
I plan eventually to support Pimoroni PicoSystem, TinyCircuits Thumby, Playdate, Thumby 2, and Pocuter PocketStar.
If you have a similar tiny programmable screen-and-buttons thing that you'd like supported, buy me one and I'll port to it.

## Protocol

USB-Serial duplex.
Multi-byte words are big-endian. (not including pixels, see Pixel Formats below).

Device (the tiny monitor) defines its framebuffer format.
Device sends input state whenever it changes.
Host (the one running the game) sends framebuffers whenever it likes.
Device is not allowed to change its format, it's expected to be constant at build time.

### Host => Device

```
0x00 : Abort. No further packets will be sent, and device should stop sending.
0x01 : Request format.
0x02 ... : Framebuffer. Must receive and understand the format first.
```

### Device => Host

```
0x00 : Abort. No further packets will be sent, and host should stop sending.
0x03 u16:w u16:h ssssqqqq : Declare format.
  (w,h) in pixels.
  (s) is log2 of pixel size in bits 1..32. 5..14 reserved. 15 => 24 bits.
  (q) is pixel format qualifier, see Pixel Formats below.
0x04 dcbadurl : Input state. (abcd) are generic buttons, use them in order.
```

## Pixel Format

In all formats, straight zeroes is black and straight ones is white.
Additionally, the zero qualifier always means straight gray big-endian.
If the Host wants to produce black-and-white images, it need not understand the qualifier.
Device must ignore the alpha channel if present, everything is opaque.

Framebuffers are always packed LRTB with rows padded to 8 bits.
Even for wonky devices like Thumby with interleaved framebuffers; that's the device's problem, not the protocol's.

```
0x00 : y1be
0x10 : y2be
0x20 : y4be
0x21 : cga4be
0x30 : y8
0x31 : bgr332 (tiny)
0x40 : y16be
0x41 : xrgb4444be (pico)
0x42 : bgr565be (tiny)
0x50 : y32be
0x51 : rgbx
0x52 : bgrx
0x53 : xrgb
0x54 : xbgr
0x55..0xef : reserved
0xf0 : y24be
0xf1 : rgb
0xf2 : bgr
```

cga4be [https://en.wikipedia.org/wiki/Color_Graphics_Adapter]:
| ix | Name          | RGB    |
|----|---------------|--------|
| 0  | black         | 000000 |
| 1  | blue          | 0000AA |
| 2  | green         | 00AA00 |
| 3  | cyan          | 00AAAA |
| 4  | red           | AA0000 |
| 5  | magenta       | AA00AA |
| 6  | brown         | AA5500 |
| 7  | light gray    | AAAAAA |
| 8  | dark gray     | 555555 |
| 9  | light blue    | 5555FF |
| 10 | light green   | 55FF55 |
| 11 | light cyan    | 55FFFF |
| 12 | light red     | FF5555 |
| 13 | light magenta | FF55FF |
| 14 | yellow        | FFFF55 |
| 15 | white         | FFFFFF |
