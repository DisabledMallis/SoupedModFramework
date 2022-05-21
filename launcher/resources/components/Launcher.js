import LaunchButton from './LaunchButton.js'
import Settings from './Settings.js'

export default {
	components: {
		LaunchButton,
        Settings
	},
	data() {
		return {
		}
	},
	methods: {
        onResize() {
        },
        isOnLauncher() {
            try {
                return areWeNative();
            } catch {
                console.log("We are not on native!");
                return false;
            }
        }
    },
	template: `
		<div class="content">
			<header>
				<h1>SoupedModFramework</h1>
			</header>
			<div class="body">
				<Settings/>
			</div>
			<footer>
				<LaunchButton />
			</footer>
		</div>`
}