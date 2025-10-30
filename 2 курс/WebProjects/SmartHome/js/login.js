function validateForm(event) {
    event.preventDefault();
    const emailInput = document.getElementById('login-email');
    const passwordInput = document.getElementById('login-password');
    const email = emailInput.value.trim();
    const password = passwordInput.value;
    const errorMessage = emailInput.nextElementSibling || document.createElement('span');
    errorMessage.className = 'error-message';
    if (!errorMessage.parentNode) emailInput.parentNode.appendChild(errorMessage);

    const emailPattern = /^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.(ru|com)$/i;

    if (!emailPattern.test(email)) {
        errorMessage.textContent = 'Пожалуйста, введите корректный email с доменом .ru или .com';
        emailInput.parentNode.classList.add('invalid');
        emailInput.focus();
        return false;
    } else {
        errorMessage.textContent = '';
        emailInput.parentNode.classList.remove('invalid');
    }

    if (password.length < 1) {
        alert('Пароль не может быть пустым');
        passwordInput.focus();
        return false;
    }

    alert(`Вход выполнен. Email: ${email}, Пароль: ${password}`);
    form.reset();
    return true;
}