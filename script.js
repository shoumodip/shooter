function colorFromHex(color) {
  return "#" + (color >>> 0).toString(16).padStart(8, "0")
}

window.onload = async () => {
  document.body.style.margin = 0
  document.body.style.padding = 0

  const app = document.getElementById("app")
  const ctx = app.getContext("2d")

  const keys = new Set()
  window.onkeyup = (e) => keys.delete(e.key)
  window.onkeydown = (e) => keys.add(e.key)

  let mouse = { x: 0, y: 0 }
  let clicked = false
  window.onmouseup = () => clicked = false
  window.onmousedown = () => clicked = true
  window.onmousemove = (e) => mouse = { x: e.offsetX, y: e.offsetY }

  const wasm = await WebAssembly.instantiateStreaming(fetch("shooter.wasm"), {
    env: {
      platformMouseX: () => {
        return mouse.x
      },

      platformMouseY: () => {
        return mouse.y
      },

      platformClicked: () => {
        return clicked
      },

      platformKeyDown: (key) => {
        return keys.has(String.fromCharCode(key))
      },

      platformDrawRect: (x, y, w, h, color) => {
        ctx.fillStyle = colorFromHex(color)
        ctx.fillRect(x, y, w, h)
      },

      platformDrawCircle: (x, y, r, color) => {
        ctx.beginPath()
        ctx.arc(x, y, r, 0, 2 * Math.PI)
        ctx.fillStyle = colorFromHex(color)
        ctx.fill()
      }
    }
  })

  ctx.canvas.width = window.innerWidth
  ctx.canvas.height = window.innerHeight
  wasm.instance.exports.gameInit(window.innerWidth, window.innerHeight)

  window.onresize = () => {
    ctx.canvas.width = window.innerWidth
    ctx.canvas.height = window.innerHeight
    wasm.instance.exports.gameResize(ctx.canvas.width, ctx.canvas.height)
  }

  function loop() {
    wasm.instance.exports.gameRender()
    wasm.instance.exports.gameUpdate()
    window.requestAnimationFrame(loop)
  }

  window.requestAnimationFrame(loop)
}
