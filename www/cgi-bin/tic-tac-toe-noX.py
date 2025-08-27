#!/usr/bin/env python3

print("Content-type: text/html")
print()

print("""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>⭕ Tic Tac Toe (O Only) - Prophet Web Server</title>
    <link rel="stylesheet" href="/css/style.css">
    <style>
        .game-board {
            display: grid;
            grid-template-columns: repeat(3, 80px);
            grid-template-rows: repeat(3, 80px);
            gap: 5px;
            justify-content: center;
            margin: 30px auto;
            background: #9b59b6;
            padding: 10px;
            border-radius: 10px;
            box-shadow: 0 10px 20px rgba(0,0,0,0.2);
        }
        
        .cell {
            background: white;
            border: none;
            border-radius: 5px;
            font-size: 2em;
            font-weight: bold;
            cursor: pointer;
            transition: all 0.3s ease;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        
        .cell:hover {
            background: #f8f9fa;
            transform: scale(1.05);
        }
        
        .cell.o {
            color: #9b59b6;
            background: #ecf0f1;
        }
        
        .game-status {
            text-align: center;
            font-size: 1.2em;
            margin: 20px 0;
            min-height: 30px;
            color: #9b59b6;
        }
        
        .winner {
            background: linear-gradient(45deg, #8e44ad, #9b59b6);
            color: white;
            padding: 10px 20px;
            border-radius: 25px;
            display: inline-block;
            animation: pulse 1s infinite;
        }
        
        @keyframes pulse {
            0%, 100% { transform: scale(1); }
            50% { transform: scale(1.05); }
        }
        
        .o-theme {
            background: linear-gradient(135deg, #e8d5f2, #f3e7f7);
        }
    </style>
</head>
<body class="o-theme">
    <div class="container fade-in">
        <!-- Navigation -->
        <nav class="nav">
            <div class="logo">🚀 Prophet Server</div>
            <ul class="nav-links">
                <li><a href="/">Home</a></li>
                <li><a href="/about.html">About</a></li>
                <li><a href="/cgi-bin/login.py">Login</a></li>
                <li><a href="/upload/uploadFile.html">Upload</a></li>
            </ul>
        </nav>

        <!-- Main Content -->
        <h1>⭕ Tic Tac Toe (O Only Mode)</h1>
        
        <div class="text-center mb-3">
            <span class="status highlight">🔮 Special O-Only Version</span>
            <span class="highlight">Only O marks allowed!</span>
        </div>

        <div class="card">
            <div class="game-status" id="status">
                Click any cell to place an O!
            </div>
            
            <div class="game-board" id="board">
                <button class="cell" onclick="makeMove(this)"></button>
                <button class="cell" onclick="makeMove(this)"></button>
                <button class="cell" onclick="makeMove(this)"></button>
                <button class="cell" onclick="makeMove(this)"></button>
                <button class="cell" onclick="makeMove(this)"></button>
                <button class="cell" onclick="makeMove(this)"></button>
                <button class="cell" onclick="makeMove(this)"></button>
                <button class="cell" onclick="makeMove(this)"></button>
                <button class="cell" onclick="makeMove(this)"></button>
            </div>
            
            <div class="text-center">
                <button onclick="resetGame()" class="button secondary">🔄 Clear Board</button>
            </div>
        </div>

        <div class="card-grid">
            <div class="card">
                <h3>⭕ Special Rules</h3>
                <ul class="feature-list">
                    <li>Only O marks can be placed</li>
                    <li>No winning conditions - just for fun!</li>
                    <li>Fill the board with beautiful O's</li>
                    <li>Perfect for testing or demos</li>
                </ul>
            </div>
            
            <div class="card">
                <h3>🎨 Board Stats</h3>
                <p><strong>O's Placed:</strong> <span id="o-count">0</span></p>
                <p><strong>Empty Cells:</strong> <span id="empty-count">9</span></p>
                <p><strong>Board Full:</strong> <span id="full-status">No</span></p>
                <p><strong>Clicks:</strong> <span id="click-count">0</span></p>
            </div>
        </div>

        <!-- Quick Actions -->
        <div class="text-center mt-3">
            <a href="/" class="button secondary">🏠 Home</a>
            <a href="/cgi-bin/tic-tac-toe.py" class="button">🎯 Full Game</a>
            <a href="/cgi-bin/hello_process.py" class="button">👋 Hello Demo</a>
        </div>

        <!-- Footer -->
        <div class="footer">
            <p>Special O-only mode powered by Prophet Web Server CGI</p>
        </div>
    </div>

    <script>
        let oCount = 0;
        let clickCount = 0;

        function makeMove(cell) {
            clickCount++;
            
            if (cell.textContent === '') {
                cell.textContent = 'O';
                cell.classList.add('o');
                cell.disabled = true;
                oCount++;
                
                updateStatus();
                updateStats();
            }
        }

        function updateStatus() {
            const emptyCount = 9 - oCount;
            
            if (oCount === 9) {
                document.getElementById('status').innerHTML = 
                    `<span class="winner">🎉 Board Complete! All O's placed!</span>`;
            } else {
                document.getElementById('status').innerHTML = 
                    `Place more O's! (${emptyCount} cells remaining)`;
            }
        }

        function updateStats() {
            const emptyCount = 9 - oCount;
            
            document.getElementById('o-count').textContent = oCount;
            document.getElementById('empty-count').textContent = emptyCount;
            document.getElementById('full-status').textContent = emptyCount === 0 ? 'Yes' : 'No';
            document.getElementById('click-count').textContent = clickCount;
        }

        function resetGame() {
            oCount = 0;
            
            const cells = document.querySelectorAll('.cell');
            cells.forEach(cell => {
                cell.textContent = '';
                cell.classList.remove('o');
                cell.disabled = false;
            });
            
            document.getElementById('status').innerHTML = 'Click any cell to place an O!';
            updateStats();
        }
    </script>
</body>
</html>""")
