```lua
(global) v.scale: string|number|"fill"|"fit"|"height"...(+4) = 'fill'|'optimal'
```

---

Fixed scale for images in viewer and slideshow modes

---

Scale of the image as a preset or absolute value

---

```lua
-- Fixed scale for images in viewer and slideshow modes
fixed_scale_t:
    | "optimal" -- 100% or less to fit to window
    | "width" -- Fit image width to window width
    | "height" -- Fit image height to window height
    | "fit" -- Fit to window
    | "fill" -- Crop image to fill the window
    | "real" -- Real size (100%)
    | "keep" -- Keep the same scale as for previously viewed image
```
