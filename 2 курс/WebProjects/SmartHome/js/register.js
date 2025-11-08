// js/register.js
document.addEventListener('DOMContentLoaded', () => {
    const form = document.getElementById('register-form');

    form.addEventListener('submit', function (e) {
        e.preventDefault();

        const emailInput = document.getElementById('register-email');
        const passwordInput = document.getElementById('register-password');
        const confirmInput = document.getElementById('confirm-password');
        const phoneInput = document.getElementById('register-phone');

        const email = emailInput.value.trim();
        const password = passwordInput.value;
        const confirm = confirmInput.value;
        const phone = phoneInput.value.trim();

        [emailInput, passwordInput, confirmInput, phoneInput].forEach(input => {
            input.parentNode.classList.remove('invalid');
            const error = input.nextElementSibling;
            if (error) error.textContent = '';
        });

        const emailPattern = /^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$/i;
        const phonePattern = /^\+7\d{10}$/;

        if (!emailPattern.test(email)) {
            showError(emailInput, 'Неверный email');
            return;
        }

        if (password.length < 6) {
            showError(passwordInput, 'Пароль от 6 символов');
            return;
        }

        if (password !== confirm) {
            showError(confirmInput, 'Пароли не совпадают');
            return;
        }

        if (!phonePattern.test(phone)) {
            showError(phoneInput, 'Формат: +79991234567');
            return;
        }

        const users = JSON.parse(localStorage.getItem('users') || '[]');
        if (users.some(u => u.email === email)) {
            showError(emailInput, 'Этот email уже занят');
            return;
        }

        users.push({ email, password, phone, name: '' });
        localStorage.setItem('users', JSON.stringify(users));

        localStorage.setItem('currentUser', JSON.stringify({ email, name: '', phone }));

        showNotification('Регистрация успешна! Вы вошли.');
        setTimeout(() => window.location.href = 'profile.html', 1000);
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