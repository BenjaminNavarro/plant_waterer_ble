import type { CapacitorConfig } from '@capacitor/cli'

const config: CapacitorConfig = {
  appId: 'io.myhome.tinydrop',
  appName: 'TinyDrop',
  webDir: 'www/dist',
  plugins: {
    EdgeToEdge: {
      backgroundColor: "#14161a",
    },
  },
}

export default config
