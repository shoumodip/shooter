function colorFromHex(color) {
  return "#" + (color >>> 0).toString(16).padStart(8, "0")
}

window.onload = async () => {
  const font = await new FontFace("Iosevka", "url(assets/november.ttf)").load()
  document.fonts.add(font)

  document.body.style.margin = 0
  document.body.style.padding = 0

  const app = document.getElementById("app")
  const ctx = app.getContext("2d")

  const keys = { down: new Set(), pressed: new Set() }
  window.onkeyup = (e) => {
    keys.pressed.add(e.key)
    keys.down.delete(e.key)
  }

  window.onkeydown = (e) => {
    keys.down.add(e.key)
    keys.pressed.delete(e.key)
  }

  let mouse = { x: 0, y: 0, down: false, pressed: false }
  window.onmouseup = () => {
    mouse.pressed = true
    mouse.down = false
  }

  window.onmousedown = () => {
    mouse.down = true
    mouse.pressed = false
  }

  window.onmousemove = (e) => {
    mouse.x = e.offsetX
    mouse.y = e.offsetY
  }

  window.onpointerup = window.onmouseup
  window.onpointerdown = window.onmousedown
  window.onpointermove = window.onmousemove

  const wasm = await WebAssembly.instantiateStreaming(fetch("assets/shooter.wasm"), {
    env: {
      platformMouseX: () => {
        return mouse.x
      },

      platformMouseY: () => {
        return mouse.y
      },

      platformMouseDown: () => {
        return mouse.down
      },

      platformMousePressed: () => {
        if (mouse.pressed) {
          mouse.pressed = false
          return true
        }
        return false
      },

      platformKeyDown: (key) => {
        return keys.down.has(String.fromCharCode(key))
      },

      platformKeyPressed: (key) => {
        const ch = String.fromCharCode(key)
        if (keys.pressed.has(ch)) {
          keys.pressed.delete(ch)
          return true
        }
        return false
      },

      platformDrawRect: (x, y, w, h, color) => {
        ctx.fillStyle = colorFromHex(color)
        ctx.fillRect(x, y, w, h)
      },

      platformDrawText: (x, y, size, ptr, color) => {
        const buf = wasm.instance.exports.memory.buffer
        const len = new Uint8Array(buf, ptr).indexOf(0)
        const src = new Uint8Array(buf, ptr, len)
        const text = new TextDecoder().decode(src)

        ctx.font = size + "px Iosevka"
        ctx.fillStyle = colorFromHex(color)

        const metrics = ctx.measureText(text)
        const offset = (metrics.actualBoundingBoxAscent + metrics.actualBoundingBoxDescent)

        ctx.fillText(text, x, y + offset)
      },

      platformDrawCircle: (x, y, r, color) => {
        ctx.beginPath()
        ctx.arc(x, y, r, 0, 2 * Math.PI)
        ctx.fillStyle = colorFromHex(color)
        ctx.fill()
      }
    }
  })

  window.onresize = () => {
    ctx.canvas.width = window.innerWidth
    ctx.canvas.height = window.innerHeight
    wasm.instance.exports.gameResize(ctx.canvas.width, ctx.canvas.height)
  }

  wasm.instance.exports.gameInit()
  window.onresize()

  function loop() {
    wasm.instance.exports.gameRender()
    wasm.instance.exports.gameUpdate()
    window.requestAnimationFrame(loop)
  }

  window.requestAnimationFrame(loop)
}
