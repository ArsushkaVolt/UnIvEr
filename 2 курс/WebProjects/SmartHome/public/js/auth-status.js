// js/auth-status.js
document.addEventListener('DOMContentLoaded', () => {
    const authLink = document.querySelector('a[href="login.html"]');
    const logoutLink = document.getElementById('logout-link');
    const currentUser = JSON.parse(localStorage.getItem('currentUser'));

    if (!authLink) return;

    if (currentUser && currentUser.email) {
        authLink.textContent = `Личный кабинет (${currentUser.email})`;
        authLink.href = 'profile.html';
        authLink.classList.add('user-account');

        if (logoutLink) {
            logoutLink.style.display = 'inline';
        }
    } else {
        authLink.textContent = 'Личный кабинет';
        authLink.href = 'login.html';
        authLink.classList.remove('user-account');

        if (logoutLink) {
            logoutLink.style.display = 'none';
        }
    }

    if (logoutLink) {
        logoutLink.onclick = e => {
            e.preventDefault();
            if (confirm('Выйти из аккаунта?')) {
                localStorage.removeItem('currentUser');
                window.location.href = 'login.html';
            }
        };
    }
});