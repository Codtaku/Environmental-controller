(function () {
    var t, i, e, s, n, o, a, h, r, l, c, p, u, d, g = [].slice,
        m = {}.hasOwnProperty,
        f = function (t, i) {
            for (var e in i) m.call(i, e) && (t[e] = i[e]);

            function s() {
                this.constructor = t
            }
            return s.prototype = i.prototype, t.prototype = new s, t.__super__ = i.prototype, t
        },
        x = [].indexOf || function (t) {
            for (var i = 0, e = this.length; i < e; i++)
                if (i in this && this[i] === t) return i;
            return -1
        };
    ! function () {
        var t, i, e, s, n, o, a;
        for (e = 0, n = (a = ["ms", "moz", "webkit", "o"]).length; e < n && (o = a[e], !window.requestAnimationFrame); e++) window.requestAnimationFrame = window[o + "RequestAnimationFrame"], window.cancelAnimationFrame = window[o + "CancelAnimationFrame"] || window[o + "CancelRequestAnimationFrame"];
        t = null, s = 0, i = {}, window.requestAnimationFrame ? window.cancelAnimationFrame || (t = window.requestAnimationFrame, window.requestAnimationFrame = function (e, n) {
            var o;
            return o = ++s, t((function () {
                if (!i[o]) return e()
            }), n), o
        }, window.cancelAnimationFrame = function (t) {
            return i[t] = !0
        }) : (window.requestAnimationFrame = function (t, i) {
            var e, s, n, o;
            return e = (new Date).getTime(), o = Math.max(0, 16 - (e - n)), s = window.setTimeout((function () {
                return t(e + o)
            }), o), n = e + o, s
        }, window.cancelAnimationFrame = function (t) {
            return clearTimeout(t)
        })
    }(), d = function (t) {
        var i, e;
        for (t -= 3600 * (i = Math.floor(t / 3600)) + 60 * (e = Math.floor((t - 3600 * i) / 60)), t += "", e += ""; e.length < 2;) e = "0" + e;
        for (; t.length < 2;) t = "0" + t;
        return (i = i ? i + ":" : "") + e + ":" + t
    }, p = function () {
        var t, i, e;
        return e = (i = 1 <= arguments.length ? g.call(arguments, 0) : [])[0], t = i[1], l(e.toFixed(t))
    }, u = function (t, i) {
        var e, s, n;
        for (e in s = {}, t) m.call(t, e) && (n = t[e], s[e] = n);
        for (e in i) m.call(i, e) && (n = i[e], s[e] = n);
        return s
    }, l = function (t) {
        var i, e, s, n;
        for (s = (e = (t += "").split("."))[0], n = "", e.length > 1 && (n = "." + e[1]), i = /(\d+)(\d{3})/; i.test(s);) s = s.replace(i, "$1,$2");
        return s + n
    }, c = function (t) {
        return "#" === t.charAt(0) ? t.substring(1, 7) : t
    }, s = function (t) {
        function i() {
            return i.__super__.constructor.apply(this, arguments)
        }
        return f(i, t), i.prototype.displayScale = 1, i.prototype.forceUpdate = !0, i.prototype.setTextField = function (t, i) {
            return this.textField = t instanceof h ? t : new h(t, i)
        }, i.prototype.setMinValue = function (t, i) {
            var e, s, n, o, a;
            if (this.minValue = t, null == i && (i = !0), i) {
                for (this.displayedValue = this.minValue, a = [], s = 0, n = (o = this.gp || []).length; s < n; s++) e = o[s], a.push(e.displayedValue = this.minValue);
                return a
            }
        }, i.prototype.setOptions = function (t) {
            return null == t && (t = null), this.options = u(this.options, t), this.textField && (this.textField.el.style.fontSize = t.fontSize + "px"), this.options.angle > .5 && (this.options.angle = .5), this.configDisplayScale(), this
        }, i.prototype.configDisplayScale = function () {
            var t, i, e, s, n;
            return s = this.displayScale, !1 === this.options.highDpiSupport ? delete this.displayScale : (i = window.devicePixelRatio || 1, t = this.ctx.webkitBackingStorePixelRatio || this.ctx.mozBackingStorePixelRatio || this.ctx.msBackingStorePixelRatio || this.ctx.oBackingStorePixelRatio || this.ctx.backingStorePixelRatio || 1, this.displayScale = i / t), this.displayScale !== s && (n = this.canvas.G__width || this.canvas.width, e = this.canvas.G__height || this.canvas.height, this.canvas.width = n * this.displayScale, this.canvas.height = e * this.displayScale, this.canvas.style.width = n + "px", this.canvas.style.height = e + "px", this.canvas.G__width = n, this.canvas.G__height = e), this
        }, i.prototype.parseValue = function (t) {
            return t = parseFloat(t) || Number(t), isFinite(t) ? t : 0
        }, i
    }(r = function () {
        function t(t, e) {
            null == t && (t = !0), this.clear = null == e || e, t && i.add(this)
        }
        return t.prototype.animationSpeed = 32, t.prototype.update = function (t) {
            var i;
            return null == t && (t = !1), !(!t && this.displayedValue === this.value) && (this.ctx && this.clear && this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height), i = this.value - this.displayedValue, Math.abs(i / this.animationSpeed) <= .001 ? this.displayedValue = this.value : this.displayedValue = this.displayedValue + i / this.animationSpeed, this.render(), !0)
        }, t
    }()), h = function () {
        function t(t, i) {
            this.el = t, this.fractionDigits = i
        }
        return t.prototype.render = function (t) {
            return this.el.innerHTML = p(t.displayedValue, this.fractionDigits)
        }, t
    }(), t = function (t) {
        function i(t, e) {
            if (this.elem = t, this.text = null != e && e, i.__super__.constructor.call(this), void 0 === this.elem) throw new Error("The element isn't defined.");
            this.value = 1 * this.elem.innerHTML, this.text && (this.value = 0)
        }
        return f(i, t), i.prototype.displayedValue = 0, i.prototype.value = 0, i.prototype.setVal = function (t) {
            return this.value = 1 * t
        }, i.prototype.render = function () {
            var t;
            return t = this.text ? d(this.displayedValue.toFixed(0)) : l(p(this.displayedValue)), this.elem.innerHTML = t
        }, i
    }(r), a = function (t) {
        function i(t) {
            if (this.gauge = t, void 0 === this.gauge) throw new Error("The element isn't defined.");
            this.ctx = this.gauge.ctx, this.canvas = this.gauge.canvas, i.__super__.constructor.call(this, !1, !1), this.setOptions()
        }
        return f(i, t), i.prototype.displayedValue = 0, i.prototype.value = 0, i.prototype.options = {
            strokeWidth: .035,
            length: .1,
            color: "#000000",
            iconPath: null,
            iconScale: 1,
            iconAngle: 0
        }, i.prototype.img = null, i.prototype.setOptions = function (t) {
            if (null == t && (t = null), this.options = u(this.options, t), this.length = 2 * this.gauge.radius * this.gauge.options.radiusScale * this.options.length, this.strokeWidth = this.canvas.height * this.options.strokeWidth, this.maxValue = this.gauge.maxValue, this.minValue = this.gauge.minValue, this.animationSpeed = this.gauge.animationSpeed, this.options.angle = this.gauge.options.angle, this.options.iconPath) return this.img = new Image, this.img.src = this.options.iconPath
        }, i.prototype.render = function () {
            var t, i, e, s, n, o, a, h, r;
            if (t = this.gauge.getAngle.call(this, this.displayedValue), h = Math.round(this.length * Math.cos(t)), r = Math.round(this.length * Math.sin(t)), o = Math.round(this.strokeWidth * Math.cos(t - Math.PI / 2)), a = Math.round(this.strokeWidth * Math.sin(t - Math.PI / 2)), i = Math.round(this.strokeWidth * Math.cos(t + Math.PI / 2)), e = Math.round(this.strokeWidth * Math.sin(t + Math.PI / 2)), this.ctx.fillStyle = this.options.color, this.ctx.beginPath(), this.ctx.arc(0, 0, this.strokeWidth, 0, 2 * Math.PI, !1), this.ctx.fill(), this.ctx.beginPath(), this.ctx.moveTo(o, a), this.ctx.lineTo(h, r), this.ctx.lineTo(i, e), this.ctx.fill(), this.img) return s = Math.round(this.img.width * this.options.iconScale), n = Math.round(this.img.height * this.options.iconScale), this.ctx.save(), this.ctx.translate(h, r), this.ctx.rotate(t + Math.PI / 2 + this.options.iconAngle), this.ctx.drawImage(this.img, -s / 2, -n / 2, s, n), this.ctx.restore()
        }, i
    }(r), i = function () {
        function i() { }
        return i.store = [], i.on = function (t, i) {
            var e;
            return (e = this.prototype)[t] = i
        }, i.prototype.clear = function () {
            return this.store = []
        }, i.prototype.add = function (t) {
            return this.store.push(t)
        }, i.prototype.update = function (t) {
            var i, e, s, n, o;
            for (null == t && (t = null), n = this.store, o = [], e = 0, s = n.length; e < s; e++) i = n[e], o.push(i.update(t));
            return o
        }, i
    }(), e = function (t) {
        function i(t) {
            this.canvas = t, i.__super__.constructor.call(this), "undefined" != typeof G_vmlCanvasManager && (this.canvas = window.G_vmlCanvasManager.initElement(this.canvas)), this.ctx = this.canvas.getContext("2d"), this.gp = [new a(this)], this.setOptions()
        }
        return f(i, t), i.prototype.elem = null, i.prototype.value = [20], i.prototype.maxValue = 80, i.prototype.minValue = 0, i.prototype.displayedAngle = 0, i.prototype.displayedValue = 0, i.prototype.lineWidth = 40, i.prototype.paddingTop = .1, i.prototype.paddingBottom = .1, i.prototype.options = {
            colorStart: "#6fadcf",
            colorStop: void 0,
            gradientType: 0,
            strokeColor: "#e0e0e0",
            pointer: {
                length: .8,
                strokeWidth: .035,
                color: "#000000"
            },
            angle: .15,
            lineWidth: .44,
            radiusScale: 1,
            fontSize: 40,
            limitMax: !1,
            limitMin: !1
        }, i.prototype.setOptions = function (t) {
            var e, s, n, o, h, r, l;
            for (null == t && (t = null), i.__super__.setOptions.call(this, t), this.configPercentColors(), this.extraPadding = 0, this.options.angle < 0 && (h = Math.PI * (1 + this.options.angle), this.extraPadding = Math.sin(h)), this.availableHeight = this.canvas.height * (1 - this.paddingTop - this.paddingBottom), this.lineWidth = this.availableHeight * this.options.lineWidth, this.radius = (this.availableHeight - this.lineWidth / 2) / (1 + this.extraPadding), this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height), o = 0, r = (l = this.gp).length; o < r; o++) s = l[o], s.setOptions(this.options.pointer), s.render();
            return this.render(), this
        }, i.prototype.set = function (t) {
            var i, e, s, n, o, a, h;
            if (!(t instanceof Array) && (t = [t]), t.length > this.gp.length)
                for (i = s = 0, o = t.length - this.gp.length; 0 <= o ? s < o : s > o; i = 0 <= o ? ++s : --s) this.gp.push(new a(this)), this.gp[this.gp.length - 1].setOptions(this.options.pointer);
            for (i = e = 0, n = (h = t.slice(0, this.gp.length)).length; e < n; i = ++e) a = h[i], a = this.parseValue(a), this.options.limitMax && a > this.maxValue ? a = this.maxValue : this.options.limitMin && a < this.minValue && (a = this.minValue), this.gp[i].value = a;
            return this.value = this.gp[0].value, this.displayedValue = this.minValue, this.forceUpdate = !0, this.update()
        }, i.prototype.getAngle = function (t) {
            return (1 + this.options.angle) * Math.PI + (t - this.minValue) / (this.maxValue - this.minValue) * (1 - 2 * this.options.angle) * Math.PI
        }, i.prototype.configPercentColors = function () {
            var t, i, e, s, n, o, a;
            if (this.percentColors = null, this.options.percentColors) {
                for (this.percentColors = new Array, o = [], s = i = 0, e = (n = this.options.percentColors).length - 1; 0 <= e ? i <= e : i >= e; s = 0 <= e ? ++i : --s) t = parseInt(c(n[s][1]).substring(0, 2), 16), a = parseInt(c(n[s][1]).substring(2, 4), 16), o.push(this.percentColors[s] = {
                    pct: n[s][0],
                    color: {
                        r: t,
                        g: parseInt(c(n[s][1]).substring(4, 6), 16),
                        b: a
                    }
                });
                return o
            }
        }, i.prototype.getColorForPercentage = function (t, i) {
            var e, s, n, o, a, h, r, l;
            if (0 === t) e = this.percentColors[0].color;
            else
                for (e = this.percentColors[this.percentColors.length - 1].color, h = 0, r = (l = this.percentColors).length; h < r && (n = l[h], !(t <= n.pct)); h++) this.percentColors.length - 1 > h && (o = this.percentColors[h + 1], s = (t - n.pct) / (o.pct - n.pct), a = o.color, e = {
                    r: Math.floor(n.color.r * (1 - s) + a.r * s),
                    g: Math.floor(n.color.g * (1 - s) + a.g * s),
                    b: Math.floor(n.color.b * (1 - s) + a.b * s)
                });
            return "rgba(" + e.r + "," + e.g + "," + e.b + "," + i + ")"
        }, i.prototype.stroke = function (t) {
            return this.ctx.beginPath(), this.ctx.strokeStyle = t, this.ctx.arc(0, 0, this.radius, (1 + this.options.angle) * Math.PI, (2 - this.options.angle) * Math.PI, !1), this.ctx.lineWidth = this.lineWidth, this.ctx.stroke()
        }, i.prototype.setGradient = function (t) {
            var i, e, s, n, o;
            if (n = this.radius - this.lineWidth / 2, o = this.radius + this.lineWidth / 2, 1 === this.options.gradientType ? (i = this.ctx.createLinearGradient(0, -n, 0, -o)).addColorStop(0, t.colorStart) : 0 === this.options.gradientType && (i = this.ctx.createRadialGradient(0, 0, n, 0, 0, o)).addColorStop(0, t.colorStart), this.options.colorStop && 1 === this.options.gradientType ? i.addColorStop(1, t.colorStop) : this.options.colorStop && 0 === this.options.gradientType && i.addColorStop(1, t.colorStop), e = 1, this.percentColors) {
                for (s = 0; e < this.percentColors.length;) {
                    var a = Math.max(Math.min((this.displayedValue - this.minValue) / (this.maxValue - this.minValue), 1), 0);
                    i.addColorStop(this.percentColors[e].pct, this.getColorForPercentage(this.percentColors[e].pct, 1)), e++
                }
            }
            return i
        }, i.prototype.render = function () {
            var t, i, e, s, n, o, a, h;
            if (n = this.getAngle(this.displayedValue), this.displayedAngle !== n && (s = this.displayedAngle, this.displayedAngle = n, e = this.ctx, this.ctx.save(), this.ctx.translate(this.canvas.width / 2, this.canvas.height / 2 + (this.canvas.height * this.paddingBottom - this.extraPadding * this.radius)), this.stroke(this.options.strokeColor), this.options.gradient ? t = this.setGradient(this.options) : this.percentColors ? t = this.getColorForPercentage(Math.max(Math.min((this.displayedValue - this.minValue) / (this.maxValue - this.minValue), 1), 0), 1) : this.options.colorStop ? (t = 1 === this.options.gradientType ? this.ctx.createLinearGradient(0, 0, this.canvas.width, 0) : this.ctx.createRadialGradient(0, 0, this.radius - this.lineWidth / 2, 0, 0, this.radius + this.lineWidth / 2)).addColorStop(0, this.options.colorStart) && t.addColorStop(1, this.options.colorStop) : t = this.options.colorStart, e.beginPath(), e.strokeStyle = t, e.arc(0, 0, this.radius, s, this.displayedAngle, !1), e.lineWidth = this.lineWidth, e.stroke(), this.ctx.restore()), o = 0, a = (h = this.gp).length; o < a; o++) i = h[o], i.update(!0);
            return this.textField && this.textField.render(this)
        }, i
    }(s), n = function (i) {
        function s(t) {
            this.canvas = t, s.__super__.constructor.call(this), "undefined" != typeof G_vmlCanvasManager && (this.canvas = window.G_vmlCanvasManager.initElement(this.canvas)), this.ctx = this.canvas.getContext("2d"), this.setOptions(), this.render()
        }
        return f(s, i), s.prototype.lineWidth = 15, s.prototype.displayedValue = 0, s.prototype.value = 33, s.prototype.maxValue = 100, s.prototype.minValue = 0, s.prototype.options = {
            lineWidth: .1,
            colorStart: "#6f6f6f",
            colorStop: "#8fc0da",
            strokeColor: "#efefef",
            shadowColor: "#d5d5d5",
            angle: .35,
            radiusScale: .9
        }, s.prototype.getAngle = function (t) {
            return (1 - this.options.angle) * Math.PI + (t - this.minValue) / (this.maxValue - this.minValue) * (2 * this.options.angle - 1) * Math.PI
        }, s.prototype.render = function () {
            var t, i, e, s, n;
            return t = this.getAngle(this.displayedValue), this.textField && this.textField.render(this), this.ctx.lineCap = "butt", s = this.displayedValue, n = this.ctx, n.save(), n.translate(this.canvas.width / 2, this.canvas.height / 2), n.beginPath(), n.strokeStyle = this.options.strokeColor, n.arc(0, 0, this.radius, (1 - this.options.angle) * Math.PI, (2 * this.options.angle + (1 - this.options.angle) - 1) * Math.PI, !1), n.lineWidth = this.lineWidth, n.stroke(), this.options.shadowColor && (n.save(), n.strokeStyle = this.options.shadowColor, n.arc(0, 0, this.radius, (1 - this.options.angle) * Math.PI, t, !1), n.lineWidth = this.lineWidth, n.stroke(), n.restore()), i = this.options.colorStart, this.options.colorStop && (e = this.ctx.createRadialGradient(0, 0, this.radius - this.lineWidth, 0, 0, this.radius)).addColorStop(0, this.options.colorStart), this.options.colorStop && e.addColorStop(1, this.options.colorStop), this.options.colorStop && (i = e), n.beginPath(), n.strokeStyle = i, n.arc(0, 0, this.radius, (1 - this.options.angle) * Math.PI, t, !1), n.lineWidth = this.lineWidth, n.stroke(), n.restore()
        }, s
    }(s), o = function (t) {
        function i(t) {
            this.canvas = t, i.__super__.constructor.call(this), "undefined" != typeof G_vmlCanvasManager && (this.canvas = window.G_vmlCanvasManager.initElement(this.canvas)), this.ctx = this.canvas.getContext("2d"), this.setOptions(this.options), this.render()
        }
        return f(i, t), i.prototype.lineWidth = 15, i.prototype.displayedValue = 0, i.prototype.value = 33, i.prototype.maxValue = 100, i.prototype.minValue = 0, i.prototype.options = {
            lineWidth: .1,
            colorStart: "#6f6f6f",
            colorStop: "#8fc0da",
            strokeColor: "#efefef",
            shadowColor: "#d5d5d5",
            angle: .35,
            radiusScale: 1
        }, i.prototype.getAngle = function (t) {
            return (1 - this.options.angle) * Math.PI + (t - this.minValue) / (this.maxValue - this.minValue) * (2 * this.options.angle - 1) * Math.PI
        }, i.prototype.render = function () {
            var t, i, e, s;
            return s = this.ctx, s.save(), s.translate(this.canvas.width / 2, this.canvas.height / 2), t = this.getAngle(this.displayedValue), this.textField && this.textField.render(this), s.beginPath(), s.strokeStyle = this.options.strokeColor, s.arc(0, 0, this.radius, (1 - this.options.angle) * Math.PI, (2 * this.options.angle + (1 - this.options.angle) - 1) * Math.PI, !1), s.lineWidth = this.lineWidth, s.stroke(), this.options.shadowColor && (s.save(), s.strokeStyle = this.options.shadowColor, s.arc(0, 0, this.radius, (1 - this.options.angle) * Math.PI, t, !1), s.lineWidth = this.lineWidth, s.stroke(), s.restore()), i = this.options.colorStart, this.options.colorStop && (e = s.createRadialGradient(0, 0, this.radius - this.lineWidth, 0, 0, this.radius)).addColorStop(0, this.options.colorStart), this.options.colorStop && e.addColorStop(1, this.options.colorStop), this.options.colorStop && (i = e), s.beginPath(), s.strokeStyle = i, s.arc(0, 0, this.radius, (1 - this.options.angle) * Math.PI, t, !1), s.lineWidth = this.lineWidth, s.stroke(), s.restore()
        }, i
    }(s), window.Gauge = e, window.Donut = n, window.BaseDonut = o, window.TextRenderer = h
}).call(this);

// Global variables for UI state and data
let lastMode = -1;
let lastData = {};
let focusedTimerInputId = null;
let focusedTimerInputValue = null;
let isInitialPageLoad = true; // Flag for initial data load

// --- Tab Handling ---
function openTab(evt, tabName) {
    let i, tabcontent, tablinks;
    tabcontent = document.getElementsByClassName("tabcontent");
    for (i = 0; i < tabcontent.length; i++) {
        tabcontent[i].style.display = "none";
    }
    tablinks = document.getElementsByClassName("tablinks");
    for (i = 0; i < tablinks.length; i++) {
        tablinks[i].className = tablinks[i].className.replace(" active", "");
    }
    document.getElementById(tabName).style.display = "block";
    if (evt && evt.currentTarget) {
        evt.currentTarget.className += " active";
    } else {
        const defaultTabButton = Array.from(tablinks).find(btn => btn.textContent.trim().includes(tabName));
        if (defaultTabButton) defaultTabButton.className += " active";
    }
}

// --- Gauge Initialization ---
let gaugeTemp, gaugeHum, gaugeGas, gaugeSoil;
function initializeGauges() {
    const baseGaugeOptions = {
        angle: 0.0, lineWidth: 0.3, radiusScale: 0.9,
        pointer: { length: 0.5, strokeWidth: 0.035, color: '#333' },
        limitMax: false, limitMin: false, strokeColor: '#E0E0E0',
        generateGradient: true, highDpiSupport: true,
        renderTicks: { divisions: 5, divWidth: 1.1, divLength: 0.5, divColor: '#666', subDivisions: 3, subLength: 0.3, subWidth: 0.6, subColor: '#aaa' },
        staticLabels: { font: "11px sans-serif", labels: [], color: "#000000", fractionDigits: 0 }
    };

    gaugeTemp = new Gauge(document.getElementById('gauge-temp')).setOptions({ ...baseGaugeOptions, staticLabels: { font: "11px sans-serif", labels: [0, 10, 20, 30, 40, 50], color: "#000" }, staticZones: [{ strokeStyle: "#87CEFA", min: 0, max: 15 }, { strokeStyle: "#90EE90", min: 15, max: 25 }, { strokeStyle: "#FFD700", min: 25, max: 35 }, { strokeStyle: "#FFA500", min: 35, max: 45 }, { strokeStyle: "#FF0000", min: 45, max: 50 }] });
    gaugeTemp.minValue = 0; gaugeTemp.maxValue = 50; gaugeTemp.set(0);
    gaugeTemp.setTextField(document.getElementById('gauge-temp-val'));

    gaugeHum = new Gauge(document.getElementById('gauge-hum')).setOptions({ ...baseGaugeOptions, staticLabels: { font: "11px sans-serif", labels: [0, 25, 50, 75, 100], color: "#000" }, staticZones: [{ strokeStyle: "#CD853F", min: 0, max: 30 }, { strokeStyle: "#9ACD32", min: 30, max: 60 }, { strokeStyle: "#87CEEB", min: 60, max: 85 }, { strokeStyle: "#0000CD", min: 85, max: 100 }] });
    gaugeHum.minValue = 0; gaugeHum.maxValue = 100; gaugeHum.set(0);
    gaugeHum.setTextField(document.getElementById('gauge-hum-val'));

    gaugeGas = new Gauge(document.getElementById('gauge-gas')).setOptions({ ...baseGaugeOptions, staticLabels: { font: "11px sans-serif", labels: [0, 1000, 2000, 3000, 4095], color: "#000" }, staticZones: [{ strokeStyle: "#EAEAEA", min: 0, max: 1000 }, { strokeStyle: "#FFFF00", min: 1000, max: 2500 }, { strokeStyle: "#ADFF2F", min: 2500, max: 4095 }] });
    gaugeGas.minValue = 0; gaugeGas.maxValue = 4095; gaugeGas.set(0);
    gaugeGas.setTextField(document.getElementById('gauge-gas-val'));

    gaugeSoil = new Gauge(document.getElementById('gauge-soil')).setOptions({ ...baseGaugeOptions, staticLabels: { font: "11px sans-serif", labels: [0, 25, 50, 75, 100], color: "#000" }, staticZones: [{ strokeStyle: "#D2B48C", min: 0, max: 30 }, { strokeStyle: "#228B22", min: 30, max: 70 }, { strokeStyle: "#1E90FF", min: 70, max: 100 }] });
    gaugeSoil.minValue = 0; gaugeSoil.maxValue = 100; gaugeSoil.set(0);
    gaugeSoil.setTextField(document.getElementById('gauge-soil-val'));
}

function updateGaugeValue(gauge, value) {
    if (gauge && value !== undefined && value !== null && !isNaN(value)) {
        gauge.set(parseFloat(value));
    } else if (gauge) { gauge.set(gauge.minValue); }
}

// --- UI Update Functions ---
function updateUI(data, actionType = 'periodic') { // actionType: 'initial', 'periodic', 'add', 'remove'
    updateGaugeValue(gaugeTemp, data.temp);
    updateGaugeValue(gaugeHum, data.humidity);
    updateGaugeValue(gaugeGas, data.gas);
    updateGaugeValue(gaugeSoil, data.soil);

    let modeText = '--';
    const timerTabButton = document.getElementById('timer-tab-button');
    if (data.mode !== undefined) {
        const modes = ['AUTO', 'OFF', 'ON', 'HALF', 'TIMER'];
        modeText = modes[data.mode % 5] || 'Unknown';
        if (timerTabButton) {
            if (data.mode === 4) { timerTabButton.classList.remove('hidden'); }
            else {
                timerTabButton.classList.add('hidden');
                if (document.getElementById('Timer').style.display === 'block') { openTab(null, 'Dashboard'); }
            }
        }
    } else { if (timerTabButton) timerTabButton.classList.add('hidden'); }
    setTextContent('mode', modeText);

    updateRelayStatus('r1', data.relay1); updateRelayStatus('r2', data.relay2);
    updateRelayStatus('r3', data.relay3); updateRelayStatus('r4', data.relay4);

    updateSettingDisplay('min-temp-disp', data.min_temp, 'min-temp-input');
    updateSettingDisplay('max-temp-disp', data.max_temp, 'max-temp-input');
    updateSettingDisplay('delay-disp', data.delay, 'delay-input');
    updateSettingDisplay('hum-limit-disp', data.hum_limit, 'hum-limit-input');
    updateSettingDisplay('gas-limit-disp', data.gas_limit, 'gas-limit-input');

    const modeSelect = document.getElementById('mode-select');
    if (modeSelect && document.activeElement !== modeSelect && data.mode !== undefined) {
        modeSelect.value = data.mode;
    }

    const forceTimerInputPopulation = (actionType === 'initial' || actionType === 'add' || actionType === 'remove');
    if (data.mode === 4 || document.getElementById('Timer').style.display === 'block') {
        setTextContent('timer-current-time', data.currentTimeStr || '--:--:--');
        setTextContent('timer-start-date', data.startDateStr || '--/--/--');
        setTextContent('timer-days-passed', data.daysPassed !== undefined ? data.daysPassed : '--');
        updateTimerTable(data.timerEntries || [], forceTimerInputPopulation);
    }

    setTextContent('nav-ip', data.ip_address ? `IP: ${data.ip_address}` : 'IP: N/A');
    lastData = data;
    if (actionType === 'initial') isInitialPageLoad = false;
}

function updateTimerTable(timerEntries, forceInputPopulation) {
    const tableBody = document.getElementById("timer-list-table").querySelector('tbody');
    if (!tableBody) return;

    let currentFocusedInputId = null;
    let currentFocusedValue = null;
    if (document.activeElement && document.activeElement.classList.contains('timer-input')) {
        currentFocusedInputId = document.activeElement.id;
        currentFocusedValue = document.activeElement.value;
    }

    const existingInputValues = {};
    if (!forceInputPopulation) { // If periodic update, store current input values
        tableBody.querySelectorAll('.timer-input').forEach(input => {
            existingInputValues[input.id] = input.value;
        });
    }

    tableBody.innerHTML = "";

    if (!timerEntries || timerEntries.length === 0) {
        tableBody.innerHTML = '<tr><td colspan="4" style="text-align:center;">No timer schedule entries found.</td></tr>';
        return;
    }

    timerEntries.forEach((entry, index) => {
        const row = tableBody.insertRow();
        row.insertCell().textContent = entry.offset !== undefined ? entry.offset : '?';

        const minInputId = `timer-min-input-${index}`;
        const maxInputId = `timer-max-input-${index}`;

        let minInputValue = (entry.min !== undefined ? entry.min : '');
        let maxInputValue = (entry.max !== undefined ? entry.max : '');

        if (!forceInputPopulation) { // Periodic update: try to keep existing value unless focused
            if (currentFocusedInputId !== minInputId && existingInputValues.hasOwnProperty(minInputId)) {
                minInputValue = existingInputValues[minInputId];
            }
            if (currentFocusedInputId !== maxInputId && existingInputValues.hasOwnProperty(maxInputId)) {
                maxInputValue = existingInputValues[maxInputId];
            }
        }
        // If it was focused, currentFocusedValue will be restored later.
        if (currentFocusedInputId === minInputId) minInputValue = currentFocusedValue;
        if (currentFocusedInputId === maxInputId) maxInputValue = currentFocusedValue;


        const minTempCell = row.insertCell();
        minTempCell.className = 'timer-input-cell';
        minTempCell.innerHTML = `<div>
                                                <span class="timer-value-display" id="timer-min-disp-${index}">${entry.min !== undefined ? entry.min : '--'}</span>
                                                <input type="number" class="timer-input" id="${minInputId}" value="${minInputValue}" step="1">
                                            </div>`;

        const maxTempCell = row.insertCell();
        maxTempCell.className = 'timer-input-cell';
        maxTempCell.innerHTML = `<div>
                                                <span class="timer-value-display" id="timer-max-disp-${index}">${entry.max !== undefined ? entry.max : '--'}</span>
                                                <input type="number" class="timer-input" id="${maxInputId}" value="${maxInputValue}" step="1">
                                            </div>`;

        const actionCell = row.insertCell();
        actionCell.innerHTML = `<button onclick="saveTimerEntry(${index})" title="Save Entry ${index + 1}"><svg width="1em" height="1em" fill="currentColor"><use xlink:href="#icon-save"></use></svg></button>
                                           <button onclick="resetTimerEntry(${index})" title="Reset Entry ${index + 1} to Defaults" style="background-color: #ffc107;"><svg width="1em" height="1em" fill="currentColor"><use xlink:href="#icon-reset"></use></svg></button>`;
    });

    if (currentFocusedInputId) {
        const focusedElement = document.getElementById(currentFocusedInputId);
        if (focusedElement) {
            focusedElement.focus();
            // Value already set correctly if it was currentFocusedInputId
            try { focusedElement.setSelectionRange(focusedElement.value.length, focusedElement.value.length); } catch (e) { /* ignore */ }
        }
    }
}

function setTextContent(id, text) {
    const el = document.getElementById(id);
    if (el) el.textContent = text !== null && text !== undefined ? text : '--';
}

function updateSettingDisplay(baseDispId, value, inputId) {
    const dispElement = document.getElementById(baseDispId); // Corrected to use baseDispId directly
    if (dispElement) dispElement.textContent = (value !== undefined && value !== null) ? value : '--';

    const inputElement = document.getElementById(inputId);
    if (inputElement && document.activeElement !== inputElement) {
        inputElement.value = (value !== undefined && value !== null) ? value : '';
    }
}

function updateRelayStatus(elementId, status) {
    const el = document.getElementById(elementId);
    if (el) {
        if (status !== undefined && status !== null) {
            el.textContent = status ? 'ON' : 'OFF';
            el.className = status ? 'value' : 'value off';
        } else {
            el.textContent = '--'; el.className = 'value warn';
        }
    }
}

function displayError(message) {
    const errEl = document.getElementById('error-msg');
    if (errEl) {
        errEl.textContent = message;
        errEl.style.display = message ? 'block' : 'none';
        if (message) { setTimeout(() => { displayError(''); }, 7000); } // Increased timeout
    }
}

const UPDATE_INTERVAL = 2500;
async function fetchStatus(actionType = 'periodic') {
    try {
        const response = await fetch('/status');
        if (!response.ok) {
            let errorText = `HTTP error! Status: ${response.status}`;
            try { errorText = await response.text() || errorText; } catch (e) { }
            throw new Error(errorText);
        }
        const data = await response.json();
        updateUI(data, actionType);
        if (actionType !== 'initial') displayError(''); // Clear error on successful subsequent fetches
    } catch (error) {
        console.error('Error fetching status:', error);
        displayError(`Connection Error: ${error.message}. Retrying...`);
    }
}

async function sendCommand(url, isPost = false, params = {}) {
    try {
        displayError('');
        let fullUrl = url;
        const options = {};
        if (isPost) {
            options.method = 'POST';
            options.headers = { 'Content-Type': 'application/x-www-form-urlencoded' };
            options.body = new URLSearchParams(params).toString();
        } else {
            if (Object.keys(params).length > 0) {
                fullUrl += '?' + new URLSearchParams(params).toString();
            }
            options.method = 'GET';
        }
        const response = await fetch(fullUrl, options);
        if (!response.ok) {
            const errorText = await response.text();
            throw new Error(`Command failed: ${response.status} ${errorText || 'No error details'}`);
        }
        const result = await response.text();
        console.log('Command result for', url, ':', result);
        // Determine actionType for fetchStatus based on URL or a new parameter if needed
        let action = 'periodic';
        if (url.includes('addtimer') || url.includes('removelasttimer')) {
            action = url.includes('addtimer') ? 'add' : 'remove';
        }
        await fetchStatus(action);
    } catch (error) {
        console.error('Error sending command:', error);
        displayError(`Command Error: ${error.message}`);
    }
}

window.setMode = function () {
    const selectedMode = document.getElementById('mode-select').value;
    sendCommand(`/setmode`, false, { mode: selectedMode });
}

window.saveAllSettings = function () {
    const minTemp = document.getElementById('min-temp-input').value;
    const maxTemp = document.getElementById('max-temp-input').value;
    const delay = document.getElementById('delay-input').value;
    const humLimit = document.getElementById('hum-limit-input').value;
    const gasLimit = document.getElementById('gas-limit-input').value;

    if (minTemp === '' || maxTemp === '' || delay === '' || humLimit === '' || gasLimit === '') { displayError("All setting fields must have values."); return; }
    const minVal = parseInt(minTemp); const maxVal = parseInt(maxTemp);
    const delayVal = parseInt(delay); const humVal = parseInt(humLimit); const gasVal = parseInt(gasLimit);
    if (isNaN(minVal) || isNaN(maxVal) || isNaN(delayVal) || isNaN(humVal) || isNaN(gasVal)) { displayError("All settings must be valid numbers."); return; }
    if (minVal > maxVal) { displayError("Min Temp cannot be greater than Max Temp."); return; }
    if (delayVal < 1 || delayVal > 300) { displayError("Switch Delay must be between 1 and 300."); return; }
    if (humVal < 0 || humVal > 100) { displayError("Humidity Limit must be between 0 and 100."); return; }
    if (gasVal < 0 || gasVal > 4095) { displayError("Gas Limit must be between 0 and 4095."); return; }

    sendCommand(`/setsettings`, false, { min: minVal, max: maxVal, delay: delayVal, hum: humVal, gas: gasVal });
}

window.syncTimeNow = function () { sendCommand('/synctime'); }

window.saveTimerEntry = function (index) {
    const minInput = document.getElementById(`timer-min-input-${index}`);
    const maxInput = document.getElementById(`timer-max-input-${index}`);
    let minValStr = minInput.value.trim(); let maxValStr = maxInput.value.trim();
    if (minValStr === "") { displayError(`Entry ${index + 1}: Min Temp cannot be empty.`); return; }
    if (maxValStr === "") { displayError(`Entry ${index + 1}: Max Temp cannot be empty.`); return; }
    const minTemp = parseInt(minValStr); const maxTemp = parseInt(maxValStr);
    if (isNaN(minTemp)) { displayError(`Entry ${index + 1}: Min Temp must be a valid number.`); return; }
    if (isNaN(maxTemp)) { displayError(`Entry ${index + 1}: Max Temp must be a valid number.`); return; }
    if (minTemp < 0 || minTemp > 50) { displayError(`Entry ${index + 1}: Min Temp must be between 0 and 50.`); return; }
    if (maxTemp < 0 || maxTemp > 50) { displayError(`Entry ${index + 1}: Max Temp must be between 0 and 50.`); return; }
    if (minTemp > maxTemp) { displayError(`Entry ${index + 1}: Min Temp (${minTemp}) cannot be greater than Max Temp (${maxTemp}).`); return; }
    sendCommand(`/settimerentry`, false, { index: index, min: minTemp, max: maxTemp });
}

window.resetTimerEntry = function (index) {
    if (!confirm(`Are you sure you want to reset Timer Entry ${index + 1} to defaults (28/32)?`)) { return; }
    sendCommand(`/resettimerentry`, false, { index: index });
}

window.addTimerEntry = function () {
    if (!confirm("Are you sure you want to add a new timer entry with default values?")) return;
    sendCommand('/addtimer'); // ESP32 will handle logic and save
}

window.removeLastTimerEntry = function () {
    if (!confirm("Are you sure you want to remove the last timer entry? This cannot be undone from the Web UI.")) return;
    sendCommand('/removelasttimer'); // ESP32 will handle logic and save
}

// --- Initialization ---
window.onload = () => {
    openTab({ currentTarget: document.querySelector('.tablinks.active') }, 'Dashboard');
    initializeGauges();
    fetchStatus('initial'); // Pass 'initial' for the first load
    setInterval(fetchStatus, UPDATE_INTERVAL);
};