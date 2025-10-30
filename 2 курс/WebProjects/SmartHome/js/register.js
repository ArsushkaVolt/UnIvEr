// register.js
document.addEventListener('DOMContentLoaded', () => {
    const form = document.getElementById('register-form');

    function validateForm(event) {
        event.preventDefault(); // Предотвращаем стандартную отправку формы
        const emailInput = document.getElementById('register-email');
        const passwordInput = document.getElementById('register-password');
        const confirmPasswordInput = document.getElementById('confirm-password');
        const phoneInput = document.getElementById('register-phone');
        const email = emailInput.value.trim();
        const password = passwordInput.value;
        const confirmPassword = confirmPasswordInput.value;
        const phone = phoneInput.value.trim();

        // Регулярные выражения
        const emailPattern = /^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.(ru|com)$/i;
        const phonePattern = /^\+7\d{10}$/; // +7 и 10 цифр

        // Очистка предыдущих ошибок
        [emailInput, passwordInput, confirmPasswordInput, phoneInput].forEach(input => {
            input.parentNode.classList.remove('invalid');
            const error = input.nextElementSibling;
            if (error) error.textContent = '';
        });

        // Валидация email
        if (!emailPattern.test(email)) {
            const error = emailInput.nextElementSibling || document.createElement('span');
            error.className = 'error-message';
            if (!error.parentNode) emailInput.parentNode.appendChild(error);
            error.textContent = 'Пожалуйста, введите корректный email с доменом .ru или .com';
            emailInput.parentNode.classList.add('invalid');
            emailInput.focus();
            return false;
        }

        // Валидация пароля
        if (password.length < 6) {
            const error = passwordInput.nextElementSibling || document.createElement('span');
            error.className = 'error-message';
            if (!error.parentNode) passwordInput.parentNode.appendChild(error);
            error.textContent = 'Пароль должен содержать минимум 6 символов';
            passwordInput.parentNode.classList.add('invalid');
            passwordInput.focus();
            return false;
        }

        // Валидация подтверждения пароля
        if (password !== confirmPassword) {
            const error = confirmPasswordInput.nextElementSibling || document.createElement('span');
            error.className = 'error-message';
            if (!error.parentNode) confirmPasswordInput.parentNode.appendChild(error);
            error.textContent = 'Пароли не совпадают';
            confirmPasswordInput.parentNode.classList.add('invalid');
            confirmPasswordInput.focus();
            return false;
        }

        // Валидация номера телефона
        if (!phonePattern.test(phone)) {
            const error = phoneInput.nextElementSibling || document.createElement('span');
            error.className = 'error-message';
            if (!error.parentNode) phoneInput.parentNode.appendChild(error);
            error.textContent = 'Пожалуйста, введите корректный номер в формате +79991234567';
            phoneInput.parentNode.classList.add('invalid');
            phoneInput.focus();
            return false;
        }

        // Успешная валидация
        alert(`Регистрация успешна. Email: ${email}, Телефон: ${phone}, Пароль: ${password}`);
        form.reset(); // Очистка формы
        return true;
    }

    form.addEventListener('submit', validateForm);
});