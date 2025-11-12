// server/routes/auth.js
import express from 'express';
import db from '../db.js';
import bcrypt from 'bcrypt';

const router = express.Router();
const saltRounds = 10;

// Регистрация
router.post('/register', async (req, res) => {
    const { email, password, phone } = req.body;
    if (!email || !password || !phone) return res.status(400).json({ error: 'Заполните все поля' });

    const hashed = await bcrypt.hash(password, saltRounds);
    try {
        const [result] = await db.execute(
            'INSERT INTO users (email, password, phone) VALUES (?, ?, ?)',
            [email, hashed, phone]
        );
        res.json({ user: { id: result.insertId, email, phone, name: '' } });
    } catch (err) {
        if (err.code === 'ER_DUP_ENTRY') return res.status(400).json({ error: 'Email уже занят' });
        res.status(500).json({ error: err.message });
    }
});

// Вход
router.post('/login', async (req, res) => {
    const { email, password } = req.body;
    try {
        const [rows] = await db.execute('SELECT * FROM users WHERE email = ?', [email]);
        if (rows.length === 0) return res.status(400).json({ error: 'Неверный email или пароль' });

        const user = rows[0];
        const match = await bcrypt.compare(password, user.password);
        if (!match) return res.status(400).json({ error: 'Неверный email или пароль' });

        res.json({
            user: {
                id: user.id,
                email: user.email,
                name: user.name,
                phone: user.phone
            }
        });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

export default router;