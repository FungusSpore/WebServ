#!/usr/bin/env python3

print("Content-type: text/html\r\n\r\n")
print("""
<html>
<head><title>Tic Tac Toe</title></head>
<body style="text-align:center; font-family:Arial;">
<h1>Tic Tac Toe</h1>
<table border="1" style="margin:auto; border-collapse:collapse;">
  <tr>
    <td onclick="mark(this)" style="width:60px;height:60px;text-align:center;font-size:30px;"></td>
    <td onclick="mark(this)" style="width:60px;height:60px;text-align:center;font-size:30px;"></td>
    <td onclick="mark(this)" style="width:60px;height:60px;text-align:center;font-size:30px;"></td>
  </tr>
  <tr>
    <td onclick="mark(this)"></td><td onclick="mark(this)"></td><td onclick="mark(this)"></td>
  </tr>
  <tr>
    <td onclick="mark(this)"></td><td onclick="mark(this)"></td><td onclick="mark(this)"></td>
  </tr>
</table>
<p id="status"></p>
<script>
let turn = "X";
function mark(cell) {
  if (cell.innerText === "") {
    cell.innerText = turn;
    turn = (turn === "X") ? "O" : "X";
    document.getElementById("status").innerText = "Turn: " + turn;
  }
}
</script>
</body>
</html>
""")
