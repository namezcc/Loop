import { createApp } from 'vue'
// import App from './App.vue'
import App from './gtest.vue'
import elplus from 'element-plus'
import 'element-plus/dist/index.css'
import './assets/css/bootstrap.min.css'

const app = createApp(App)
app.use(elplus)
app.mount('#app')
