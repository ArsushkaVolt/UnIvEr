// public/js/api.js
const API_URL = 'http://localhost:3000/api';

async function api(path, options = {}) {
    const res = await fetch(`${API_URL}${path}`, {
        headers: { 'Content-Type': 'application/json' },
        ...options
    });
    const data = await res.json();
    if (!res.ok) throw new Error(data.error || 'Ошибка сервера');
    return data;
}

// Авторизация
export const register = (email, password, phone) =>
    api('/auth/register', { method: 'POST', body: JSON.stringify({ email, password, phone }) });

export const login = (email, password) =>
    api('/auth/login', { method: 'POST', body: JSON.stringify({ email, password }) });

// Корзина
export const addToCart = (user_id, product_name, price, quantity = 1) =>
    api('/cart', { method: 'POST', body: JSON.stringify({ user_id, product_name, price, quantity }) });

export const getCart = (user_id) => api(`/cart/${user_id}`);
export const removeFromCart = (id) => api(`/cart/${id}`, { method: 'DELETE' });

// Заказы
export const createOrder = (user_id, items, total, address) =>
    api('/orders', { method: 'POST', body: JSON.stringify({ user_id, items, total, address }) });

export const getOrders = (user_id) => api(`/orders/${user_id}`);