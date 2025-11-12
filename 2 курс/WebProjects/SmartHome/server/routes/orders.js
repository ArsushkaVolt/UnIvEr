// server/routes/orders.js
import express from 'express';
import db from '../db.js';

const router = express.Router();

// Создать заказ
router.post('/', async (req, res) => {
    const { user_id, items, total, address } = req.body;
    const connection = await db.getConnection();
    try {
        await connection.beginTransaction();

        const [orderResult] = await connection.execute(
            'INSERT INTO orders (user_id, total, address) VALUES (?, ?, ?)',
            [user_id, total, address]
        );
        const orderId = orderResult.insertId;

        const stmt = connection.prepare(
            'INSERT INTO order_items (order_id, product_name, price, quantity) VALUES (?, ?, ?, ?)'
        );
        for (const item of items) {
            await stmt.execute([orderId, item.name, item.price, item.quantity]);
        }
        await stmt.end();

        await connection.execute('DELETE FROM cart WHERE user_id = ?', [user_id]);
        await connection.commit();

        res.json({ orderId });
    } catch (err) {
        await connection.rollback();
        res.status(500).json({ error: err.message });
    } finally {
        connection.release();
    }
});

// Получить заказы
router.get('/:user_id', async (req, res) => {
    const { user_id } = req.params;
    try {
        const [rows] = await db.execute(`
            SELECT o.*, oi.product_name, oi.price, oi.quantity
            FROM orders o
            LEFT JOIN order_items oi ON o.id = oi.order_id
            WHERE o.user_id = ?
            ORDER BY o.created_at DESC
        `, [user_id]);
        res.json(rows);
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

export default router;