// js/profile.js
document.addEventListener('DOMContentLoaded', () => {
    const currentUser = JSON.parse(localStorage.getItem('currentUser'));
    if (!currentUser) {
        alert('Пожалуйста, войдите в аккаунт');
        window.location.href = 'login.html';
        return;
    }

    const nameEl = document.getElementById('profile-name');
    const emailEl = document.getElementById('profile-email');
    const phoneEl = document.getElementById('profile-phone');
    const ordersList = document.getElementById('orders-list');
    const editBtn = document.querySelector('.edit-profile-btn');
    const modal = document.getElementById('edit-modal');
    const closeBtn = document.querySelector('.modal-close');
    const editForm = document.getElementById('edit-form');
    const logoutLink = document.getElementById('logout-link');

    function loadProfile() {
        nameEl.textContent = currentUser.name || 'Не указано';
        emailEl.textContent = currentUser.email;
        phoneEl.textContent = currentUser.phone || 'Не указано';
        loadOrders();
    }

    function loadOrders() {
        const orders = JSON.parse(localStorage.getItem('orders') || '[]');
        const userOrders = orders.filter(o => o.email === currentUser.email);

        if (userOrders.length === 0) {
            ordersList.innerHTML = '<p class="no-orders">У вас пока нет заказов</p>';
            return;
        }

        ordersList.innerHTML = '';
        userOrders.forEach(order => {
            const el = document.createElement('div');
            el.className = 'order-item';
            el.innerHTML = `
                <div class="order-header">
                    <strong>Заказ #${order.id}</strong>
                    <span class="order-date">${new Date(order.date).toLocaleDateString('ru-RU')}</span>
                </div>
                <div class="order-total">Итого: ${order.total.toLocaleString()} ₽</div>
                <div class="order-status ${order.status}">${getStatusText(order.status)}</div>
            `;
            ordersList.appendChild(el);
        });
    }

    function getStatusText(status) {
        const map = { pending: 'Обрабатывается', shipped: 'Отправлен', delivered: 'Доставлен' };
        return map[status] || 'Неизвестно';
    }

    editBtn.onclick = () => {
        document.getElementById('edit-name').value = currentUser.name || '';
        document.getElementById('edit-phone').value = currentUser.phone || '';
        modal.style.display = 'flex';
        document.body.style.overflow = 'hidden';
    };

    closeBtn.onclick = () => {
        modal.style.display = 'none';
        document.body.style.overflow = '';
    };

    window.onclick = e => {
        if (e.target === modal) {
            modal.style.display = 'none';
            document.body.style.overflow = '';
        }
    };

    editForm.onsubmit = e => {
        e.preventDefault();
        const name = document.getElementById('edit-name').value.trim();
        const phone = document.getElementById('edit-phone').value.trim();

        if (!/^\+7\d{10}$/.test(phone)) {
            alert('Неверный формат телефона: +79991234567');
            return;
        }

        const users = JSON.parse(localStorage.getItem('users') || '[]');
        const userIndex = users.findIndex(u => u.email === currentUser.email);
        if (userIndex !== -1) {
            users[userIndex].name = name;
            users[userIndex].phone = phone;
            localStorage.setItem('users', JSON.stringify(users));
        }

        currentUser.name = name;
        currentUser.phone = phone;
        localStorage.setItem('currentUser', JSON.stringify(currentUser));

        loadProfile();
        modal.style.display = 'none';
        document.body.style.overflow = '';
        showNotification('Профиль обновлён');
    };

    logoutLink.onclick = e => {
        e.preventDefault();
        if (confirm('Выйти из аккаунта?')) {
            localStorage.removeItem('currentUser');
            window.location.href = 'login.html';
        }
    };

    function showNotification(msg) {
        const existing = document.querySelector('.cart-notification');
        if (existing) existing.remove();

        const n = document.createElement('div');
        n.className = 'cart-notification';
        n.textContent = msg;
        document.body.appendChild(n);
        requestAnimationFrame(() => n.style.opacity = '1');
        setTimeout(() => { n.style.opacity = '0'; setTimeout(() => n.remove(), 300); }, 2500);
    }

    loadProfile();
});