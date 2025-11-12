// js/login.js
document.addEventListener('DOMContentLoaded', () => {
    const form = document.getElementById('login-form');
    const emailInput = document.getElementById('login-email');
    const passwordInput = document.getElementById('login-password');

    form.addEventListener('submit', function (e) {
        e.preventDefault();

        const email = emailInput.value.trim();
        const password = passwordInput.value;

        document.querySelectorAll('.error-message').forEach(el => el.textContent = '');
        document.querySelectorAll('.form-group').forEach(el => el.classList.remove('invalid'));

        const emailPattern = /^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$/i;

        if (!emailPattern.test(email)) {
            showError(emailInput, 'Неверный формат email');
            return;
        }

        if (!password) {
            showError(passwordInput, 'Введите пароль');
            return;
        }

        const users = JSON.parse(localStorage.getItem('users') || '[]');
        const user = users.find(u => u.email === email);

        if (!user) {
            showError(emailInput, 'Пользователь не найден');
            return;
        }

        if (user.password !== password) {
            showError(passwordInput, 'Неверный пароль');
            return;
        }

        localStorage.setItem('currentUser', JSON.stringify({
            email: user.email,
            name: user.name || '',
            phone: user.phone || ''
        }));

        showNotification('Вход выполнен');
        setTimeout(() => window.location.href = 'profile.html', 800);
    });

    function showError(input, message) {
        const error = input.nextElementSibling || document.createElement('span');
        error.className = 'error-message';
        error.textContent = message;
        if (!error.parentNode) input.parentNode.appendChild(error);
        input.parentNode.classList.add('invalid');
        input.focus();
    }

    function showNotification(msg) {
        const n = document.createElement('div');
        n.className = 'cart-notification';
        n.textContent = msg;
        document.body.appendChild(n);
        requestAnimationFrame(() => n.style.opacity = '1');
        setTimeout(() => { n.style.opacity = '0'; setTimeout(() => n.remove(), 300); }, 2000);
    }
});