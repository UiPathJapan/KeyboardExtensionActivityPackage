using System;
using System.Activities;

namespace UiPathTeam.KeyboardExtension.Activities
{
    [LocalizedDisplayName(nameof(Resource.SetStateDisplayName))]
    [LocalizedDescription(nameof(Resource.SetStateDescription))]
    public class SetState : CodeActivity
    {
        [LocalizedCategory(nameof(Resource.InputCategory))]
        [LocalizedDisplayName(nameof(Resource.SetStateWindowHandleDisplayName))]
        [LocalizedDescription(nameof(Resource.SetStateWindowHandleDescription))]
        [RequiredArgument]
        public InArgument<object> WindowHandle { get; set; }

        [LocalizedCategory(nameof(Resource.InputCategory))]
        [LocalizedDisplayName(nameof(Resource.SetStateKeyboardOpenCloseDisplayName))]
        [LocalizedDescription(nameof(Resource.SetStateKeyboardOpenCloseDescription))]
        public InArgument<bool> KeyboardOpenClose { get; set; }

        [LocalizedCategory(nameof(Resource.InputCategory))]
        [LocalizedDisplayName(nameof(Resource.SetStateConversionModeDisplayName))]
        [LocalizedDescription(nameof(Resource.SetStateConversionModeDescription))]
        public InArgument<int> ConversionMode { get; set; }

        [LocalizedCategory(nameof(Resource.OutputCategory))]
        [LocalizedDisplayName(nameof(Resource.SetStatePreviousKeyboardOpenCloseDisplayName))]
        [LocalizedDescription(nameof(Resource.SetStatePreviousKeyboardOpenCloseDescription))]
        public OutArgument<bool> PreviousKeyboardOpenClose { get; set; }

        [LocalizedCategory(nameof(Resource.OutputCategory))]
        [LocalizedDisplayName(nameof(Resource.SetStatePreviousConversionModeDisplayName))]
        [LocalizedDescription(nameof(Resource.SetStatePreviousConversionModeDescription))]
        public OutArgument<int> PreviousConversionMode { get; set; }

        protected override void Execute(CodeActivityContext context)
        {
            var h = WindowHandle.Get(context);
            if (h == null)
            {
                throw new ArgumentException(Resource.WindowHandleArgumentValidationError);
            }
            int state = 0;
            int mask = 0;
            if (KeyboardOpenClose.Expression != null)
            {
                if (ConversionMode.Expression != null)
                {
                    throw new ArgumentException(Resource.SetStateInputArgumentValidationError);
                }
                state |= (KeyboardOpenClose.Get(context) ? 1 : 0) << 15;
                mask |= Bridge.OPENCLOSE;
            }
            else if (ConversionMode.Expression != null)
            {
                state |= (ConversionMode.Get(context) & 0x7fff) << 0;
                mask |= Bridge.CONVERSION;
            }
            var value = Bridge.SetState(Bridge.ToWin32Handle(h), state, mask);
            PreviousKeyboardOpenClose.Set(context, ((value >> 15) & 1) == 1);
            PreviousConversionMode.Set(context, (value >> 0) & 0x7fff);
        }
    }
}
