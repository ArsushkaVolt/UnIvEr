// js/cart.js — ПОЛНАЯ РАБОЧАЯ ВЕРСИЯ
document.addEventListener('DOMContentLoaded', () => {
    const CART_STORAGE_KEY = 'smarthome_cart';
    const modal = document.getElementById('checkout-modal');
    const modalClose = document.querySelector('.modal-close');
    const checkoutBtn = document.querySelector('.checkout-btn');
    const checkoutForm = document.getElementById('checkout-form');
    const modalItems = document.getElementById('modal-cart-items');
    const modalTotal = document.getElementById('modal-total');

    function getCart() {
        const stored = localStorage.getItem(CART_STORAGE_KEY);
        return stored ? JSON.parse(stored) : [];
    }

    function saveCart(cart) {
        localStorage.setItem(CART_STORAGE_KEY, JSON.stringify(cart));
    }

    window.addToCart = function (productName, price) {
        if (!productName || !price) return;
        const cart = getCart();
        const existing = cart.find(i => i.name === productName);
        if (existing) {
            existing.quantity += 1;
        } else {
            const newId = Date.now();
            cart.push({ id: newId, name: productName, price: Number(price), quantity: 1 });
        }
        saveCart(cart);
        updateCartBadge();
        showNotification(`${productName} добавлен в корзину!`);
    };

    window.removeFromCart = function (id) {
        id = Number(id);
        let cart = getCart();
        const index = cart.findIndex(i => i.id === id);
        if (index === -1) return;
        cart.splice(index, 1);
        saveCart(cart);

        const itemEl = document.querySelector(`.cart-item[data-id="${id}"]`);
        if (itemEl) {
            itemEl.style.transition = 'all 0.3s ease';
            itemEl.style.opacity = '0';
            itemEl.style.transform = 'translateX(-20px)';
            setTimeout(() => { updateCartPage(); updateCartBadge(); }, 300);
        } else {
            updateCartPage();
            updateCartBadge();
        }
    };

    window.updateQuantity = function (id, change) {
        id = Number(id);
        const cart = getCart();
        const item = cart.find(i => i.id === id);
        if (item) {
            item.quantity = Math.max(1, item.quantity + change);
            saveCart(cart);
            updateCartPage();
            updateCartBadge();
        }
    };

    function updateCartPage() {
        const cart = getCart();
        const container = document.querySelector('.cart-items');
        if (!container) return;
        container.innerHTML = '';

        if (cart.length === 0) {
            container.innerHTML = `<div class="cart-empty"><p>Корзина пуста</p><a href="products.html" class="btn">Перейти к товарам</a></div>`;
            if (checkoutBtn) checkoutBtn.disabled = true;
            return;
        }

        let total = 0;
        cart.forEach(item => {
            const itemTotal = item.price * item.quantity;
            total += itemTotal;
            const el = document.createElement('div');
            el.className = 'cart-item';
            el.dataset.id = item.id;
            el.innerHTML = `
                <div class="cart-item-info">
                    <span class="cart-item-name">${item.name}</span>
                    <div class="cart-item-controls">
                        <button onclick="updateQuantity(${item.id}, -1)" ${item.quantity <= 1 ? 'disabled' : ''}>−</button>
                        <span class="quantity">${item.quantity}</span>
                        <button onclick="updateQuantity(${item.id}, 1)">+</button>
                    </div>
                </div>
                <div class="cart-item-price">
                    <span>${itemTotal.toLocaleString()} ₽</span>
                    <button class="remove-item" onclick="removeFromCart(${item.id})">×</button>
                </div>
            `;
            container.appendChild(el);
        });

        const totalEl = document.createElement('div');
        totalEl.className = 'cart-total';
        totalEl.innerHTML = `<strong>Итого: ${total.toLocaleString()} ₽</strong>`;
        container.appendChild(totalEl);

        if (checkoutBtn) {
            checkoutBtn.disabled = false;
            checkoutBtn.onclick = openCheckoutModal;
        }
    }

    function openCheckoutModal() {
        const cart = getCart();
        let total = 0;
        modalItems.innerHTML = '';
        cart.forEach(item => {
            const itemTotal = item.price * item.quantity;
            total += itemTotal;
            const el = document.createElement('div');
            el.className = 'modal-cart-item';
            el.innerHTML = `<span>${item.name} × ${item.quantity}</span> <strong>${itemTotal.toLocaleString()} ₽</strong>`;
            modalItems.appendChild(el);
        });
        modalTotal.textContent = total.toLocaleString();
        modal.style.display = 'flex';
        document.body.style.overflow = 'hidden';
    }

    function closeModal() {
        modal.style.display = 'none';
        document.body.style.overflow = '';
        checkoutForm?.reset();
        document.querySelectorAll('.checkout-form .error-message').forEach(e => e.textContent = '');
        document.querySelectorAll('.checkout-form .form-group').forEach(g => g.classList.remove('invalid'));
    }

    if (modalClose) modalClose.onclick = closeModal;
    window.onclick = e => { if (e.target === modal) closeModal(); };

    if (checkoutForm) {
        checkoutForm.onsubmit = function (e) {
            e.preventDefault();
            const name = document.getElementById('name').value.trim();
            const phone = document.getElementById('phone').value.trim();
            const email = document.getElementById('email').value.trim();
            const address = document.getElementById('address').value.trim();

            let valid = true;
            document.querySelectorAll('.checkout-form .error-message').forEach(e => e.textContent = '');
            document.querySelectorAll('.checkout-form .form-group').forEach(g => g.classList.remove('invalid'));

            if (!name) { showModalError('name', 'Введите ФИО'); valid = false; }
            if (!/^\+7\d{10}$/.test(phone)) { showModalError('phone', 'Формат: +79991234567'); valid = false; }
            if (!/^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(email)) { showModalError('email', 'Неверный email'); valid = false; }
            if (!address) { showModalError('address', 'Введите адрес'); valid = false; }

            if (!valid) return;

            showNotification('Заказ успешно оформлен! Мы свяжемся с вами.');
            localStorage.removeItem(CART_STORAGE_KEY);
            closeModal();
            updateCartPage();
            updateCartBadge();
        };
    }

    function showModalError(field, message) {
        const input = document.getElementById(field);
        const error = input.nextElementSibling;
        error.textContent = message;
        input.parentNode.classList.add('invalid');
    }

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

    function updateCartBadge() {
        const badge = document.querySelector('.cart-badge');
        if (!badge) return;
        const total = getCart().reduce((s, i) => s + i.quantity, 0);
        badge.textContent = total;
        badge.style.display = total > 0 ? 'inline-flex' : 'none';
    }

    const cartLink = document.querySelector('a[href="cart.html"]');
    if (cartLink) {
        let badge = cartLink.querySelector('.cart-badge');
        if (!badge) {
            badge = document.createElement('span');
            badge.className = 'cart-badge';
            cartLink.style.position = 'relative';
            cartLink.appendChild(badge);
        }
        updateCartBadge();
    }

    document.querySelectorAll('.add-to-cart').forEach(btn => {
        if (btn.dataset.initialized) return;
        const name = btn.dataset.name || btn.closest('.product-card, .product-info')?.querySelector('h3')?.textContent;
        const price = btn.dataset.price || btn.closest('.product-card, .product-info')?.querySelector('p')?.textContent?.match(/\d[\d\s]*/)?.[0]?.replace(/\s/g, '');
        if (name && price) {
            btn.dataset.initialized = 'true';
            btn.addEventListener('click', e => { e.preventDefault(); e.stopPropagation(); addToCart(name, price); });
        }
    });

    if (window.location.pathname.includes('cart.html')) updateCartPage();
    updateCartBadge();
});