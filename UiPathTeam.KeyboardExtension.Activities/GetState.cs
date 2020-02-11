using System;
using System.Activities;

namespace UiPathTeam.KeyboardExtension.Activities
{
    [LocalizedDisplayName(nameof(Resource.GetStateDisplayName))]
    [LocalizedDescription(nameof(Resource.GetStateDescription))]
    public class GetState : CodeActivity
    {
        [LocalizedCategory(nameof(Resource.InputCategory))]
        [LocalizedDisplayName(nameof(Resource.GetStateWindowHandleDisplayName))]
        [LocalizedDescription(nameof(Resource.GetStateWindowHandleDescription))]
        [RequiredArgument]
        public InArgument<object> WindowHandle { get; set; }

        [LocalizedCategory(nameof(Resource.OutputCategory))]
        [LocalizedDisplayName(nameof(Resource.GetStateKeyboardLayoutDisplayName))]
        [LocalizedDescription(nameof(Resource.GetStateKeyboardLayoutDescription))]
        public OutArgument<int> KeyboardLayout { get; set; }

        [LocalizedCategory(nameof(Resource.OutputCategory))]
        [LocalizedDisplayName(nameof(Resource.GetStateKeyboardOpenCloseDisplayName))]
        [LocalizedDescription(nameof(Resource.GetStateKeyboardOpenCloseDescription))]
        public OutArgument<bool> KeyboardOpenClose { get; set; }

        [LocalizedCategory(nameof(Resource.OutputCategory))]
        [LocalizedDisplayName(nameof(Resource.GetStateConversionModeDisplayName))]
        [LocalizedDescription(nameof(Resource.GetStateConversionModeDescription))]
        public OutArgument<int> ConversionMode { get; set; }

        protected override void Execute(CodeActivityContext context)
        {
            var h = WindowHandle.Get(context);
            if (h == null)
            {
                throw new ArgumentException(Resource.WindowHandleArgumentValidationError);
            }
            var value = Bridge.SetState(Bridge.ToWin32Handle(h), 0, 0);
            KeyboardLayout.Set(context, (value >> 16) & 0xffff);
            KeyboardOpenClose.Set(context, ((value >> 15) & 1) == 1);
            ConversionMode.Set(context, (value >> 0) & 0x7fff);
        }
    }
}
