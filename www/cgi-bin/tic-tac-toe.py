#!/usr/bin/env python3

print("Content-type: text/html")
print()

print("""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>🎯 Tic Tac Toe - Prophet Web Server</title>
    <link rel="stylesheet" href="/css/style.css">
    <style>
        .game-board {
            display: grid;
            grid-template-columns: repeat(3, 80px);
            grid-template-rows: repeat(3, 80px);
            gap: 5px;
            justify-content: center;
            margin: 30px auto;
            background: #34495e;
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
            background: #ecf0f1;
            transform: scale(1.05);
        }
        
        .cell.x {
            color: #e74c3c;
        }
        
        .cell.o {
            color: #3498db;
        }
        
        .game-status {
            text-align: center;
            font-size: 1.2em;
            margin: 20px 0;
            min-height: 30px;
        }
        
        .turn-x {
            color: #e74c3c;
        }
        
        .turn-o {
            color: #3498db;
        }
        
        .winner {
            background: linear-gradient(45deg, #27ae60, #2ecc71);
            color: white;
            padding: 10px 20px;
            border-radius: 25px;
            display: inline-block;
            animation: bounce 0.5s ease-in-out;
        }
        
        @keyframes bounce {
            0%, 100% { transform: translateY(0); }
            50% { transform: translateY(-10px); }
        }
    </style>
</head>
<body>
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
        <h1>🎯 Tic Tac Toe Game</h1>
        
        <div class="text-center mb-3">
            <span class="status running">🎮 Game Active</span>
            <span class="highlight">Interactive JavaScript Game</span>
        </div>

        <div class="card">
            <div class="game-status" id="status">
                <span class="turn-x">Player X's Turn</span>
            </div>
            
            <div class="game-board" id="board">
                <button class="cell" onclick="makeMove(this, 0)"></button>
                <button class="cell" onclick="makeMove(this, 1)"></button>
                <button class="cell" onclick="makeMove(this, 2)"></button>
                <button class="cell" onclick="makeMove(this, 3)"></button>
                <button class="cell" onclick="makeMove(this, 4)"></button>
                <button class="cell" onclick="makeMove(this, 5)"></button>
                <button class="cell" onclick="makeMove(this, 6)"></button>
                <button class="cell" onclick="makeMove(this, 7)"></button>
                <button class="cell" onclick="makeMove(this, 8)"></button>
            </div>
            
            <div class="text-center">
                <button onclick="resetGame()" class="button secondary">🔄 New Game</button>
            </div>
        </div>

        <div class="card-grid">
            <div class="card">
                <h3>🎮 How to Play</h3>
                <ul class="feature-list">
                    <li>Click any empty cell to place your mark</li>
                    <li>X always goes first</li>
                    <li>Get 3 in a row to win!</li>
                    <li>Use the "New Game" button to restart</li>
                </ul>
            </div>
            
            <div class="card">
                <h3>🔧 Game Stats</h3>
                <p><strong>Player X Wins:</strong> <span id="x-wins">0</span></p>
                <p><strong>Player O Wins:</strong> <span id="o-wins">0</span></p>
                <p><strong>Draws:</strong> <span id="draws">0</span></p>
                <p><strong>Games Played:</strong> <span id="total-games">0</span></p>
            </div>
        </div>

        <!-- Quick Actions -->
        <div class="text-center mt-3">
            <a href="/" class="button secondary">🏠 Home</a>
            <a href="/cgi-bin/hello_process.py" class="button">👋 Hello Demo</a>
            <a href="/cgi-bin/tic-tac-toe-noX.py" class="button">⭕ No X Version</a>
        </div>

        <!-- Footer -->
        <div class="footer">
            <p>Interactive game powered by Prophet Web Server CGI + JavaScript</p>
        </div>
    </div>

    <script>
        let currentPlayer = 'X';
        let gameBoard = ['', '', '', '', '', '', '', '', ''];
        let gameActive = true;
        let stats = { xWins: 0, oWins: 0, draws: 0, totalGames: 0 };

        function makeMove(cell, index) {
            if (gameBoard[index] === '' && gameActive) {
                gameBoard[index] = currentPlayer;
                cell.textContent = currentPlayer;
                cell.classList.add(currentPlayer.toLowerCase());
                cell.disabled = true;
                
                if (checkWinner()) {
                    document.getElementById('status').innerHTML = 
                        `<span class="winner">🎉 Player ${currentPlayer} Wins!</span>`;
                    stats[currentPlayer.toLowerCase() + 'Wins']++;
                    stats.totalGames++;
                    gameActive = false;
                } else if (gameBoard.every(cell => cell !== '')) {
                    document.getElementById('status').innerHTML = 
                        `<span class="winner">🤝 It's a Draw!</span>`;
                    stats.draws++;
                    stats.totalGames++;
                    gameActive = false;
                } else {
                    currentPlayer = currentPlayer === 'X' ? 'O' : 'X';
                    document.getElementById('status').innerHTML = 
                        `<span class="turn-${currentPlayer.toLowerCase()}">Player ${currentPlayer}'s Turn</span>`;
                }
                
                updateStats();
            }
        }

        function checkWinner() {
            const winPatterns = [
                [0, 1, 2], [3, 4, 5], [6, 7, 8], // rows
                [0, 3, 6], [1, 4, 7], [2, 5, 8], // columns
                [0, 4, 8], [2, 4, 6] // diagonals
            ];
            
            return winPatterns.some(pattern => {
                const [a, b, c] = pattern;
                return gameBoard[a] && gameBoard[a] === gameBoard[b] && gameBoard[a] === gameBoard[c];
            });
        }

        function resetGame() {
            currentPlayer = 'X';
            gameBoard = ['', '', '', '', '', '', '', '', ''];
            gameActive = true;
            
            const cells = document.querySelectorAll('.cell');
            cells.forEach(cell => {
                cell.textContent = '';
                cell.classList.remove('x', 'o');
                cell.disabled = false;
            });
            
            document.getElementById('status').innerHTML = 
                `<span class="turn-x">Player X's Turn</span>`;
        }

        function updateStats() {
            document.getElementById('x-wins').textContent = stats.xWins;
            document.getElementById('o-wins').textContent = stats.oWins;
            document.getElementById('draws').textContent = stats.draws;
            document.getElementById('total-games').textContent = stats.totalGames;
        }
    </script>
</body>
</html>""")
