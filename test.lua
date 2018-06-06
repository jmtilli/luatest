t = {}
t["foo"] = "bar"
h = Http2.req("www.google.fi", t, true)
t2 = Http2.headers(t)
print("--")
print(t2["foo"])
print(t2["bar"])
print("--")

t = Http2.datablk(h)
while true do
  print(t.data)
  if t.last then
    break
  end
  t = Http2.datablk(h)
end

Http2.free(h)
