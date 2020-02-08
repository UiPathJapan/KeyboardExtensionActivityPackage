using System.Activities;

namespace UiPathTeam.KeyboardExtension.Activities
{
    [LocalizedDisplayName(nameof(Resource.ConfigureDisplayName))]
    [LocalizedDescription(nameof(Resource.ConfigureDescription))]
    public class Configure : CodeActivity
    {
        private static readonly string DEFAULT_TOGGLE_SEQUENCE = "LCONTROL+LSHIFT+RSHIFT";
        private const ushort DEFAULT_PREFERRED_KEYBOARD_LAYOUT = 0x409; // ENGLISH/US

        [LocalizedCategory(nameof(Resource.InputCategory))]
        [LocalizedDisplayName(nameof(Resource.BlockKeyboardInputDisplayName))]
        [LocalizedDescription(nameof(Resource.BlockKeyboardInputDescription))]
        public bool BlockKeyboardInput { get; set; }

        [LocalizedCategory(nameof(Resource.InputCategory))]
        [LocalizedDisplayName(nameof(Resource.BlockMouseInputDisplayName))]
        [LocalizedDescription(nameof(Resource.BlockMouseInputDescription))]
        public bool BlockMouseInput { get; set; }

        [LocalizedCategory(nameof(Resource.InputCategory))]
        [LocalizedDisplayName(nameof(Resource.DisableInputMethodEditorDisplayName))]
        [LocalizedDescription(nameof(Resource.DisableInputMethodEditorDescription))]
        public bool DisableInputMethodEditor { get; set; }

        [LocalizedCategory(nameof(Resource.InputCategory))]
        [LocalizedDisplayName(nameof(Resource.ForceKeyboardLayoutDisplayName))]
        [LocalizedDescription(nameof(Resource.ForceKeyboardLayoutDescription))]
        public bool ForceKeyboardLayout { get; set; }

        [LocalizedCategory(nameof(Resource.InputCategory))]
        [LocalizedDisplayName(nameof(Resource.ToggleSequenceDisplayName))]
        [LocalizedDescription(nameof(Resource.ToggleSequenceDescription))]
        public InArgument<string> ToggleSequence { get; set; }

        [LocalizedCategory(nameof(Resource.InputCategory))]
        [LocalizedDisplayName(nameof(Resource.PreferredKeyboardLayoutDisplayName))]
        [LocalizedDescription(nameof(Resource.PreferredKeyboardLayoutDescription))]
        public InArgument<ushort> PreferredKeyboardLayout { get; set; }

        public Configure()
        {
            DisableInputMethodEditor = true;
            ForceKeyboardLayout = false;
            BlockKeyboardInput = false;
            BlockMouseInput = false;
            ToggleSequence = new InArgument<string>(DEFAULT_TOGGLE_SEQUENCE);
            PreferredKeyboardLayout = new InArgument<ushort>(DEFAULT_PREFERRED_KEYBOARD_LAYOUT);
        }

        protected override void Execute(CodeActivityContext context)
        {
            int flagsToSet = 0;
            int flagsToReset = 0;

            if (BlockKeyboardInput)
            {
                flagsToSet |= Bridge.FLAG_DISABLE_KEYBD;
            }
            else
            {
                flagsToReset |= Bridge.FLAG_DISABLE_KEYBD;
            }

            if (BlockMouseInput)
            {
                flagsToSet |= Bridge.FLAG_DISABLE_MOUSE;
            }
            else
            {
                flagsToReset |= Bridge.FLAG_DISABLE_MOUSE;
            }

            if (DisableInputMethodEditor)
            {
                flagsToSet |= Bridge.FLAG_DISABLE_IME;
            }
            else
            {
                flagsToReset |= Bridge.FLAG_DISABLE_IME;
            }

            if (ForceKeyboardLayout)
            {
                flagsToSet |= Bridge.FLAG_FORCE_LAYOUT;
            }
            else
            {
                flagsToReset |= Bridge.FLAG_FORCE_LAYOUT;
            }

            Bridge.SetFlags(flagsToSet, flagsToReset);

            if (ToggleSequence != null)
            {
                Bridge.SetToggleSequence(ToggleSequence.Get(context));
            }

            ushort langId = PreferredKeyboardLayout != null ? PreferredKeyboardLayout.Get(context) : (ushort)0;
            if (langId == 0)
            {
                langId = DEFAULT_PREFERRED_KEYBOARD_LAYOUT;
            }
            Bridge.SetPreferredKeyboardLayout(langId);
        }
    }
}
