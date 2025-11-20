// BrainForge Complete Frontend Logic
let username = localStorage.getItem('bf_user') || '';
// let currentLevel = 1; // Removed this confusing variable. We use window.currentLevel
let currentScore = 0;
let totalScore = 0;
let questions = [];
let answered = {};

// Initialize theme
(function initTheme() {
  const theme = localStorage.getItem('bf_theme') || 'dark';
  document.body.className = theme;
  const icon = document.getElementById('theme-icon');
  if (icon) icon.textContent = theme === 'dark' ? '☀️' : '🌙';
})();

// Theme toggle
function toggleTheme() {
  const body = document.body;
  const isDark = body.classList.contains('dark');
  body.className = isDark ? 'light' : 'dark';
  localStorage.setItem('bf_theme', isDark ? 'light' : 'dark');
  const icon = document.getElementById('theme-icon');
  if (icon) icon.textContent = isDark ? '🌙' : '☀️';
}

// Toast notification
function showToast(message, duration = 2500) {
  const toast = document.getElementById('toast');
  if (!toast) return;
  toast.textContent = message;
  toast.classList.add('show');
  setTimeout(() => toast.classList.remove('show'), duration);
}

// Navigation functions
function backHome() {
  window.location.href = '/index.html';
}

function goPlay() {
  if (!username) {
    showToast('Please login first!');
    openAuth();
    return;
  }
  window.location.href = '/game.html';
}

// *** MODIFICATION: This function now also clears the in-memory 'username' ***
function exitApp() {
  if (confirm('Are you sure you want to exit?')) {
    localStorage.removeItem('bf_user');
    username = ''; // <-- Clear the in-memory variable
    showToast('Goodbye!');
    setTimeout(() => window.location.href = '/index.html', 1000);
  }
}

// Auth Modal
function openAuth() {
  const modal = document.getElementById('auth-modal');
  if (modal) {
    modal.style.display = 'flex';
    const input = document.getElementById('username');
    if (input) input.focus();
  }
}

// *** MODIFICATION: This function now clears the input field ***
function closeAuth() {
  const modal = document.getElementById('auth-modal');
  if (modal) modal.style.display = 'none';
  
  const msg = document.getElementById('auth-message');
  if (msg) msg.textContent = '';

  // <-- Clear the input field -->
  const input = document.getElementById('username');
  if (input) input.value = '';
}

function register() {
  const input = document.getElementById('username');
  const name = input ? input.value.trim() : '';
  
  if (!name) {
    showToast('Please enter a username');
    return;
  }
  
  if (name.length < 3) {
    showToast('Username must be at least 3 characters');
    return;
  }
  
  fetch('/api/register', {
    method: 'POST',
    body: 'name=' + encodeURIComponent(name)
  })
  .then(r => r.json())
  .then(data => {
    if (data.status === 'ok') {
      username = name;
      localStorage.setItem('bf_user', username);
      showToast('Registration successful! Welcome to Brain Forge!');
      closeAuth();
    } else {
      showToast(data.message || 'Registration failed');
    }
  })
  .catch(() => showToast('Network error. Please try again.'));
}

function login() {
  const input = document.getElementById('username');
  const name = input ? input.value.trim() : '';
  
  if (!name) {
    showToast('Please enter a username');
    return;
  }
  
  fetch('/api/login', {
    method: 'POST',
    body: 'name=' + encodeURIComponent(name)
  })
  .then(r => r.json())
  .then(data => {
    if (data.status === 'ok') {
      username = name;
      localStorage.setItem('bf_user', username);
      showToast('Login successful! Welcome back!');
      closeAuth();
    } else {
      showToast(data.message || 'User not found. Please register first.');
    }
  })
  .catch(() => showToast('Network error. Please try again.'));
}

// Rules Modal
function toggleRulesModal() {
  const modal = document.getElementById('rules-modal');
  if (modal) {
    modal.style.display = modal.style.display === 'flex' ? 'none' : 'flex';
  }
}

// Leaderboard
function showLeaderboard() {
  const modal = document.getElementById('leaderboard-modal');
  const list = document.getElementById('leaderboard-list');
  
  if (modal) modal.style.display = 'flex';
  if (list) list.innerHTML = '<p>Loading...</p>';
  
  fetch('/api/leaderboard')
    .then(r => r.json())
    .then(data => {
      if (!list) return;
      if (data.length === 0) {
        list.innerHTML = '<p>No players yet. Be the first!</p>';
        return;
      }
      
      let html = '<ol>';
      data.forEach((player, i) => {
        const medal = i === 0 ? '🥇' : i === 1 ? '🥈' : i === 2 ? '🥉' : '';
        html += `<li>${medal} <strong>${player.username}</strong> - ${player.score} points</li>`;
      });
      html += '</ol>';
      list.innerHTML = html;
    })
    .catch(() => {
      if (list) list.innerHTML = '<p>Failed to load leaderboard</p>';
    });
}

function closeLeaderboard() {
  const modal = document.getElementById('leaderboard-modal');
  if (modal) modal.style.display = 'none';
}

// Game Logic
function startLevel() {
  if (!username) {
    showToast('Please login first!');
    return;
  }
  
  const level = window.currentLevel || 1;
  resetGame();
  
  if (level === 3) {
    loadLevel3();
    return;
  }
  
  const gameArea = document.getElementById('game-area');
  if (gameArea) gameArea.innerHTML = '<p class="loading">Loading questions...</p>';
  
  fetch('/api/questions', {
    method: 'POST',
    body: `name=${encodeURIComponent(username)}&level=${level}`
  })
  .then(r => r.json())
  .then(data => {
    if (data.error) {
      showToast(data.error);
      if (gameArea) gameArea.innerHTML = `<p class="error">${data.error}</p>`;
      return;
    }
    
    questions = data;
    renderQuestions(data);
    document.getElementById('finish-btn').disabled = false;
  })
  .catch(err => {
    showToast('Failed to load questions');
    if (gameArea) gameArea.innerHTML = '<p class="error">Failed to load questions</p>';
  });
}

function resetGame() {
  currentScore = 0;
  answered = {};
  const scoreEl = document.getElementById('current-score');
  if (scoreEl) scoreEl.textContent = '0';
  
  const gameArea = document.getElementById('game-area');
  if (gameArea) gameArea.innerHTML = '';
  
  const level3Area = document.getElementById('level3-area');
  if (level3Area) level3Area.style.display = 'none';
  
  const msgBox = document.getElementById('message-box');
  if (msgBox) msgBox.style.display = 'none';
}

function renderQuestions(questions) {
  const gameArea = document.getElementById('game-area');
  if (!gameArea) return;
  
  gameArea.innerHTML = '';
  
  questions.forEach((q, index) => {
    const qDiv = document.createElement('div');
    qDiv.className = 'question-card';
    
    const qHeader = document.createElement('div');
    qHeader.className = 'question-header';
    qHeader.innerHTML = `<strong>Question ${index + 1}:</strong> ${escapeHtml(q.q)}`;
    qDiv.appendChild(qHeader);
    
    const optionsDiv = document.createElement('div');
    optionsDiv.className = 'options';
    
    q.opts.forEach((opt, i) => {
      const btn = document.createElement('button');
      btn.className = 'option-btn';
      btn.textContent = opt;
      btn.onclick = () => submitAnswer(q.id, i, btn, optionsDiv);
      optionsDiv.appendChild(btn);
    });
    
    qDiv.appendChild(optionsDiv);
    gameArea.appendChild(qDiv);
  });
}

function submitAnswer(qid, ansIndex, btn, container) {
  if (answered[qid]) {
    showToast('Already answered this question');
    return;
  }
  
  answered[qid] = true;
  
  // Disable all buttons in this question
  const buttons = container.querySelectorAll('.option-btn');
  buttons.forEach(b => b.disabled = true);
  
  fetch('/api/submit_answer', {
    method: 'POST',
    body: `name=${encodeURIComponent(username)}&level=${window.currentLevel}&qid=${qid}&ans=${ansIndex}`
  })
  .then(r => r.json())
  .then(data => {
    if (data.correct) {
      currentScore++;
      btn.classList.add('correct');
      showToast('✓ Correct!');
    } else {
      btn.classList.add('wrong');
      showToast('✗ Wrong');
    }
    
    const scoreEl = document.getElementById('current-score');
    if (scoreEl) scoreEl.textContent = currentScore;
    
    // Check if all answered
    if (Object.keys(answered).length === questions.length) {
      showToast('All questions answered! Click Finish Level to submit.');
    }
  })
  .catch(() => {
    answered[qid] = false;
    buttons.forEach(b => b.disabled = false);
    showToast('Failed to submit answer. Try again.');
  });
}

function finishLevel() {
  const finishBtn = document.getElementById('finish-btn');
  if (finishBtn) finishBtn.disabled = true; // Disable button at the start

  // Handle Level 3 flow
  if (window.currentLevel === 3) {
    finishLevel3(); // Call the new L3 function
    return;
  }

  // --- This is the original L1/L2 flow ---
  if (Object.keys(answered).length === 0) {
    showToast('Please answer at least one question!');
    if (finishBtn) finishBtn.disabled = false; // Re-enable
    return;
  }
  
  if (Object.keys(answered).length < questions.length) {
    if (!confirm(`You've only answered ${Object.keys(answered).length} out of ${questions.length} questions. Continue?`)) {
      if (finishBtn) finishBtn.disabled = false; // Re-enable
      return;
    }
  }
  
  fetch('/api/finish_level', {
    method: 'POST',
    body: `name=${encodeURIComponent(username)}&level=${window.currentLevel}&score=${currentScore}`
  })
  .then(r => r.json())
  .then(data => {
    const msgBox = document.getElementById('message-box');
    if (msgBox) {
      msgBox.innerHTML = `<h3>${data.message}</h3><p>Your score: ${currentScore}/10</p>`;
      msgBox.style.display = 'block';
    }
    
    showToast(data.message);
    
    const totalEl = document.getElementById('total-score');
    if (totalEl) totalEl.textContent = data.total_score;
    
    if (data.next_level) {
      window.currentLevel = data.next_level;
      const levelEl = document.getElementById('current-level');
      if (levelEl) levelEl.textContent = window.currentLevel;
      
      const startBtn = document.getElementById('start-btn');
      if (startBtn) startBtn.textContent = 'Start Next Level';
    } else {
      const startBtn = document.getElementById('start-btn');
      if (startBtn) startBtn.textContent = 'Retry Level';
    }
    
    // button remains disabled
  })
  .catch(() => {
    showToast('Failed to finish level');
    if (finishBtn) finishBtn.disabled = false; // Re-enable on failure
  });
}

function loadLevel3() {
  const gameArea = document.getElementById('game-area');
  const level3Area = document.getElementById('level3-area');
  
  if (gameArea) gameArea.innerHTML = '';
  if (level3Area) {
    level3Area.style.display = 'block';
    const list = document.getElementById('level3-list');
    if (list) list.innerHTML = '<p>Loading challenges...</p>';
  }
  
  fetch('/api/level3')
    .then(r => r.json())
    .then(data => {
      const list = document.getElementById('level3-list');
      if (!list) return;
      
      let html = '<div class="problem-grid">';
      data.forEach(problem => {
        const diffClass = problem.difficulty.toLowerCase();
        html += `
          <div class="problem-card">
            <h3>${escapeHtml(problem.title)}</h3>
            <span class="difficulty ${diffClass}">${problem.difficulty}</span>
            <a href="${problem.url}" target="_blank" rel="noopener noreferrer" class="problem-link">
              Solve on LeetCode →
            </a>
          </div>
        `;
      });
      html += '</div>';
      list.innerHTML = html;
    })
    .catch(() => {
      const list = document.getElementById('level3-list');
      if (list) list.innerHTML = '<p>Failed to load challenges</p>';
    });
}

function completeLevel3() {
  if (!username) {
    showToast('Please login first!');
    return;
  }

  // Add 1 point to UI score
  currentScore = 1;
  const scoreEl = document.getElementById('current-score');
  if (scoreEl) scoreEl.textContent = currentScore;

  // Disable the "I'm Done!" button
  const btn = document.getElementById('level3-done-btn');
  if (btn) btn.disabled = true;

  // Enable the "Finish Level" button
  const finishBtn = document.getElementById('finish-btn');
  if (finishBtn) finishBtn.disabled = false;

  showToast("You've earned 1 point! Click 'Finish Level' to save.");
}

function finishLevel3() {
  fetch('/api/level3_complete', {
    method: 'POST',
    body: `name=${encodeURIComponent(username)}`
  })
  .then(r => r.json())
  .then(data => {
    if (data.status === 'ok') {
      // Show final toast
      showToast(`Game complete! Final score: ${data.total_score}. Logging you out...`, 2000);
      
      // Hide the level 3 area
      const level3Area = document.getElementById('level3-area');
      if (level3Area) level3Area.style.display = 'none';
      
      // Set a timer to redirect to home (which logs them out)
      setTimeout(redirectToHome, 2000); // 2 second delay

    } else {
      showToast(data.message || 'Failed to submit.');
      const finishBtn = document.getElementById('finish-btn');
      if (finishBtn) finishBtn.disabled = false; // Re-enable on failure
    }
  })
  .catch(() => {
    showToast('Network error. Please try again.');
    const finishBtn = document.getElementById('finish-btn');
    if (finishBtn) finishBtn.disabled = false; // Re-enable on failure
  });
}

// *** MODIFICATION: This function now also clears the global 'username' var ***
function redirectToHome() {
  localStorage.removeItem('bf_user'); // <-- This is the logout
  username = ''; // <-- Clear the in-memory variable
  window.location.href = '/index.html';
}
// *** END MODIFICATION ***

function escapeHtml(text) {
  const div = document.createElement('div');
  div.textContent = text;
  return div.innerHTML;
}

// Close modals on outside click
window.onclick = function(event) {
  const authModal = document.getElementById('auth-modal');
  const rulesModal = document.getElementById('rules-modal');
  const leaderModal = document.getElementById('leaderboard-modal');
  
  if (event.target === authModal) closeAuth();
  if (event.target === rulesModal) toggleRulesModal();
  if (event.target === leaderModal) closeLeaderboard();
};